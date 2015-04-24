void __init kmem_cache_init(void)
{
	size_t left_over;
	struct cache_sizes *sizes;
	struct cache_names *names;
	int i;
	int order;
	int node;
	/* 在slab初始化好之前，无法通过kmalloc分配初始化过程中必要的一些对象 ，只能使用静态的全局变量 
         ，待slab初始化后期，再使用kmalloc动态分配的对象替换全局变量 */  
  
       /* 如前所述，先借用全局变量initkmem_list3表示的slab三链 
        ，每个内存节点对应一组slab三链。initkmem_list3是个slab三链数组，对于每个内存节点，包含三组 
        ：struct kmem_cache的slab三链、struct arraycache_init的slab 三链、struct kmem_list3的slab三链 。
这里循环初始化所有内存节点的所有slab三链 */  
	//在UMA模型中，系统中只有一个节点，包含所有的物理内存。
	if (num_possible_nodes() == 1) {
		use_alien_caches = 0;
		numa_platform = 0;
	}

	for (i = 0; i < NUM_INIT_LISTS; i++) {//初始化所有node的所有slab中的三个链表
		kmem_list3_init(&initkmem_list3[i]);
		if (i < MAX_NUMNODES)
			cache_cache.nodelists[i] = NULL;//全局静态变量cache_cache，这个变量是用来管理所有缓存的kmem_cache的， 
也就是说，在初始化阶段，将会创建一个slab，用来存放所有缓存的kmem_cache
	}

	/*
	 * Fragmentation resistance on low memory - only use bigger
	 * page orders on machines with more than 32MB of memory.
	 */
	if (num_physpages > (32 << 20) >> PAGE_SHIFT)//num_physpages是记录系统实际存在物理内存的总页数，如果大于32M
		slab_break_gfp_order = BREAK_GFP_ORDER_HI;//才可以创建高阶指数内存页数的高速缓存内存对象

	/* Bootstrap is tricky, because several objects are allocated
	 * from caches that do not exist yet:
	 * 1) initialize the cache_cache cache: it contains the struct
	 *    kmem_cache structures of all caches, except cache_cache itself:
	 *    cache_cache is statically allocated.
	 *    Initially an __init data area is used for the head array and the
	 *    kmem_list3 structures, it's replaced with a kmalloc allocated
	 *    array at the end of the bootstrap.
	 *		初始化cache_cache的cache：cache_cache是一个静态分配的高速缓存描述符，
	 *		其所描述的高速缓存存放的是接下来所有的kmem_cache描述符。
	 * 2) Create the first kmalloc cache.
	 *    The struct kmem_cache for the new cache is allocated normally.
	 *    An __init data area is used for the head array.
	 * 3) Create the remaining kmalloc caches, with minimally sized
	 *    head arrays.
	 * 4) Replace the __init data head arrays for cache_cache and the first
	 *    kmalloc cache with kmalloc allocated arrays.
	 * 5) Replace the __init data for kmem_list3 for cache_cache and
	 *    the other cache's with kmalloc allocated memory.
	 * 6) Resize the head arrays of the kmalloc caches to their final sizes.
	 */

	node = numa_node_id();//获取当前的内存结点号

	//第一步，创建struct kmem_cache所在的cache，由全局变量cache_cache指向，这里只是初始化数据结构，并未真正创建这些
	//对象，要待分配时才创建。全局变量cache_chain是内核slab cache链表的表头 
	INIT_LIST_HEAD(&cache_chain);//初始化保存所有slab cache的全局链表cache_chain
	list_add(&cache_cache.next, &cache_chain);//将cache_cache加入到slab cache链表
	cache_cache.colour_off = cache_line_size();//设置cache着色基本单位为cache line的大小：32字节
	cache_cache.array[smp_processor_id()] = &initarray_cache.cache;
   //初始化cache_cache的local cache，同样这里也不能使用kmalloc，需要使用静态分配的全局变量initarray_cache
	cache_cache.nodelists[node] = &initkmem_list3[CACHE_CACHE];//初始化slab链表 ,用全局变量

	/*
	 * struct kmem_cache size depends on nr_node_ids, which
	 * can be less than MAX_NUMNODES.
	 */
	cache_cache.buffer_size = offsetof(struct kmem_cache, nodelists) + nr_node_ids * sizeof(struct kmem_list3 *);
   //buffer_size保存slab中对象的大小，这里是计算struct kmem_cache的大小，nodelists是最后一个成员，nr_node_ids
   //保存内存节点个数，UMA为1，所以nodelists偏移加上1个struct kmem_list3 的大小即为struct kmem_cache的大小
#if DEBUG
	cache_cache.obj_size = cache_cache.buffer_size;
#endif
	cache_cache.buffer_size = ALIGN(cache_cache.buffer_size,
					cache_line_size());//将对象大小与cache line大小对齐
	cache_cache.reciprocal_buffer_size =
		reciprocal_value(cache_cache.buffer_size);//计算对象大小的倒数，用于计算对象在slab中的索引

	for (order = 0; order < MAX_ORDER; order++) {
		cache_estimate(order, cache_cache.buffer_size,
			cache_line_size(), 0, &left_over, &cache_cache.num);//计算cache_cache中的对象数目
		if (cache_cache.num)//num不为0意味着创建struct kmem_cache对象成功，退出
			break;
	}
	BUG_ON(!cache_cache.num);
	cache_cache.gfporder = order;//gfporder表示本slab包含2^gfporder个页面
	cache_cache.colour = left_over / cache_cache.colour_off;//着色区的大小，以colour_off为单位
	cache_cache.slab_size = ALIGN(cache_cache.num * sizeof(kmem_bufctl_t) +
				      sizeof(struct slab), cache_line_size());//slab管理对象的大小

	/* 2+3) create the kmalloc caches */
	sizes = malloc_sizes;//malloc_sizes保存大小
	names = cache_names;//cache_names保存cache名

	/*
	 * Initialize the caches that provide memory for the array cache and the
	 * kmem_list3 structures first.  Without this, further allocations will
	 * bug.
	 */
	//首先创建struct array_cache和struct kmem_list3所用的general cache，它们是后续初始化动作的基础
	sizes[INDEX_AC].cs_cachep = kmem_cache_create(names[INDEX_AC].name,
				sizes[INDEX_AC].cs_size,
				ARCH_KMALLOC_MINALIGN,
				ARCH_KMALLOC_FLAGS|SLAB_PANIC,
				NULL);//INDEX_AC是计算local cache所用的struct arraycache_init对象在kmalloc size中的索引，
   //即属于哪一级别大小的general cache，创建此大小级别的cache为local cache所用

	if (INDEX_AC != INDEX_L3) {//如果struct kmem_list3和struct arraycache_init对应的kmalloc size索引不同，
   //即大小属于不同的级别，则创建struct kmem_list3所用的cache，否则共用一个cache
		sizes[INDEX_L3].cs_cachep =
			kmem_cache_create(names[INDEX_L3].name,
				sizes[INDEX_L3].cs_size,
				ARCH_KMALLOC_MINALIGN,
				ARCH_KMALLOC_FLAGS|SLAB_PANIC,
				NULL);
	}

	slab_early_init = 0;//创建完上述两个general cache后，slab early init阶段结束，在此之前，不允许创建外置式slab

	while (sizes->cs_size != ULONG_MAX) {//循环创建kmalloc各级别的general cache
		/*
		 * For performance, all the general caches are L1 aligned.
		 * This should be particularly beneficial on SMP boxes, as it
		 * eliminates "false sharing".
		 * Note for systems short on memory removing the alignment will
		 * allow tighter packing of the smaller caches.
		 */
		if (!sizes->cs_cachep) {
       //某级别的kmalloc cache还未创建，创建之，struct kmem_list3和struct arraycache_init对应的cache已经创建过了
			sizes->cs_cachep = kmem_cache_create(names->name,
					sizes->cs_size,
					ARCH_KMALLOC_MINALIGN,
					ARCH_KMALLOC_FLAGS|SLAB_PANIC,
					NULL);
		}
#ifdef CONFIG_ZONE_DMA
		sizes->cs_dmacachep = kmem_cache_create(
					names->name_dma,
					sizes->cs_size,
					ARCH_KMALLOC_MINALIGN,
					ARCH_KMALLOC_FLAGS|SLAB_CACHE_DMA|
						SLAB_PANIC,
					NULL);
#endif
		sizes++;
		names++;
	}//至此，kmalloc general cache已经创建完毕，可以拿来使用了
	/* 4) Replace the bootstrap head arrays */
	{//第四步，用kmalloc对象替换静态分配的全局变量。到目前为止一共使用了两个全局local cache，一个是cache_cache的
 	//local cache指向initarray_cache.cache，另一个是malloc_sizes[INDEX_AC].cs_cachep的local cache指向
 	//initarray_generic.cache，参见setup_cpu_cache函数。这里替换它们。
		struct array_cache *ptr;

		ptr = kmalloc(sizeof(struct arraycache_init), GFP_KERNEL);//申请cache_cache所用local cache的空间

		local_irq_disable();
		BUG_ON(cpu_cache_get(&cache_cache) != &initarray_cache.cache);
		memcpy(ptr, cpu_cache_get(&cache_cache),
		       sizeof(struct arraycache_init));//复制原cache_cache的local cache，即initarray_cache，到新的位置
		/*
		 * Do not assume that spinlocks can be initialized via memcpy:
		 */
		spin_lock_init(&ptr->lock);

		cache_cache.array[smp_processor_id()] = ptr;//cache_cache的local cache指向新的位置
		local_irq_enable();

		ptr = kmalloc(sizeof(struct arraycache_init), GFP_KERNEL);
       //申请malloc_sizes[INDEX_AC].cs_cachep所用local cache的空间

		local_irq_disable();
		BUG_ON(cpu_cache_get(malloc_sizes[INDEX_AC].cs_cachep)
		       != &initarray_generic.cache);
		memcpy(ptr, cpu_cache_get(malloc_sizes[INDEX_AC].cs_cachep),
		       sizeof(struct arraycache_init));//复制原local cache到新分配的位置，注意此时local cache的大小是固定的
		/*
		 * Do not assume that spinlocks can be initialized via memcpy:
		 */
		spin_lock_init(&ptr->lock);

		malloc_sizes[INDEX_AC].cs_cachep->array[smp_processor_id()] =
		    ptr;
		local_irq_enable();
	}
	/* 5) Replace the bootstrap kmem_list3's */
	{//第五步，与第四步类似，用kmalloc的空间替换静态分配的slab三链
		int nid;

		/* Replace the static kmem_list3 structures for the boot cpu */
		init_list(&cache_cache, &initkmem_list3[CACHE_CACHE], node);//复制struct kmem_cache的slab三链

		for_each_online_node(nid) {
			init_list(malloc_sizes[INDEX_AC].cs_cachep,
				  &initkmem_list3[SIZE_AC + nid], nid);//复制struct arraycache_init的slab三链

			if (INDEX_AC != INDEX_L3) {
				init_list(malloc_sizes[INDEX_L3].cs_cachep,
					  &initkmem_list3[SIZE_L3 + nid], nid);//复制struct kmem_list3的slab三链
			}
		}
	}

	/* 6) resize the head arrays to their final sizes */
	{//初始化阶段local cache的大小是固定的，要根据对象大小重新计算
		struct kmem_cache *cachep;
		mutex_lock(&cache_chain_mutex);
		list_for_each_entry(cachep, &cache_chain, next)
			if (enable_cpucache(cachep))
				BUG();
		mutex_unlock(&cache_chain_mutex);
	}

	/* Annotate slab for lockdep -- annotate the malloc caches */
	init_lock_keys();


	/* Done! */
	g_cpucache_up = FULL;//大功告成，general cache终于全部建立起来了

	/*
	 * Register a cpu startup notifier callback that initializes
	 * cpu_cache_get for all new cpus
	 */
	register_cpu_notifier(&cpucache_notifier);//注册cpu up回调函数，cpu up时配置local cache

	/*
	 * The reap timers are started later, with a module init call: That part
	 * of the kernel is not yet operational.
	 */
} 