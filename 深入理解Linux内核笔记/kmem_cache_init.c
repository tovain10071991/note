void __init kmem_cache_init(void)
{
	size_t left_over;
	struct cache_sizes *sizes;
	struct cache_names *names;
	int i;
	int order;
	int node;
	/* ��slab��ʼ����֮ǰ���޷�ͨ��kmalloc�����ʼ�������б�Ҫ��һЩ���� ��ֻ��ʹ�þ�̬��ȫ�ֱ��� 
         ����slab��ʼ�����ڣ���ʹ��kmalloc��̬����Ķ����滻ȫ�ֱ��� */  
  
       /* ��ǰ�������Ƚ���ȫ�ֱ���initkmem_list3��ʾ��slab���� 
        ��ÿ���ڴ�ڵ��Ӧһ��slab������initkmem_list3�Ǹ�slab�������飬����ÿ���ڴ�ڵ㣬�������� 
        ��struct kmem_cache��slab������struct arraycache_init��slab ������struct kmem_list3��slab���� ��
����ѭ����ʼ�������ڴ�ڵ������slab���� */  
	//��UMAģ���У�ϵͳ��ֻ��һ���ڵ㣬�������е������ڴ档
	if (num_possible_nodes() == 1) {
		use_alien_caches = 0;
		numa_platform = 0;
	}

	for (i = 0; i < NUM_INIT_LISTS; i++) {//��ʼ������node������slab�е���������
		kmem_list3_init(&initkmem_list3[i]);
		if (i < MAX_NUMNODES)
			cache_cache.nodelists[i] = NULL;//ȫ�־�̬����cache_cache����������������������л����kmem_cache�ģ� 
Ҳ����˵���ڳ�ʼ���׶Σ����ᴴ��һ��slab������������л����kmem_cache
	}

	/*
	 * Fragmentation resistance on low memory - only use bigger
	 * page orders on machines with more than 32MB of memory.
	 */
	if (num_physpages > (32 << 20) >> PAGE_SHIFT)//num_physpages�Ǽ�¼ϵͳʵ�ʴ��������ڴ����ҳ�����������32M
		slab_break_gfp_order = BREAK_GFP_ORDER_HI;//�ſ��Դ����߽�ָ���ڴ�ҳ���ĸ��ٻ����ڴ����

	/* Bootstrap is tricky, because several objects are allocated
	 * from caches that do not exist yet:
	 * 1) initialize the cache_cache cache: it contains the struct
	 *    kmem_cache structures of all caches, except cache_cache itself:
	 *    cache_cache is statically allocated.
	 *    Initially an __init data area is used for the head array and the
	 *    kmem_list3 structures, it's replaced with a kmalloc allocated
	 *    array at the end of the bootstrap.
	 *		��ʼ��cache_cache��cache��cache_cache��һ����̬����ĸ��ٻ�����������
	 *		���������ĸ��ٻ����ŵ��ǽ��������е�kmem_cache��������
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

	node = numa_node_id();//��ȡ��ǰ���ڴ����

	//��һ��������struct kmem_cache���ڵ�cache����ȫ�ֱ���cache_cacheָ������ֻ�ǳ�ʼ�����ݽṹ����δ����������Щ
	//����Ҫ������ʱ�Ŵ�����ȫ�ֱ���cache_chain���ں�slab cache����ı�ͷ 
	INIT_LIST_HEAD(&cache_chain);//��ʼ����������slab cache��ȫ������cache_chain
	list_add(&cache_cache.next, &cache_chain);//��cache_cache���뵽slab cache����
	cache_cache.colour_off = cache_line_size();//����cache��ɫ������λΪcache line�Ĵ�С��32�ֽ�
	cache_cache.array[smp_processor_id()] = &initarray_cache.cache;
   //��ʼ��cache_cache��local cache��ͬ������Ҳ����ʹ��kmalloc����Ҫʹ�þ�̬�����ȫ�ֱ���initarray_cache
	cache_cache.nodelists[node] = &initkmem_list3[CACHE_CACHE];//��ʼ��slab���� ,��ȫ�ֱ���

	/*
	 * struct kmem_cache size depends on nr_node_ids, which
	 * can be less than MAX_NUMNODES.
	 */
	cache_cache.buffer_size = offsetof(struct kmem_cache, nodelists) + nr_node_ids * sizeof(struct kmem_list3 *);
   //buffer_size����slab�ж���Ĵ�С�������Ǽ���struct kmem_cache�Ĵ�С��nodelists�����һ����Ա��nr_node_ids
   //�����ڴ�ڵ������UMAΪ1������nodelistsƫ�Ƽ���1��struct kmem_list3 �Ĵ�С��Ϊstruct kmem_cache�Ĵ�С
#if DEBUG
	cache_cache.obj_size = cache_cache.buffer_size;
#endif
	cache_cache.buffer_size = ALIGN(cache_cache.buffer_size,
					cache_line_size());//�������С��cache line��С����
	cache_cache.reciprocal_buffer_size =
		reciprocal_value(cache_cache.buffer_size);//��������С�ĵ��������ڼ��������slab�е�����

	for (order = 0; order < MAX_ORDER; order++) {
		cache_estimate(order, cache_cache.buffer_size,
			cache_line_size(), 0, &left_over, &cache_cache.num);//����cache_cache�еĶ�����Ŀ
		if (cache_cache.num)//num��Ϊ0��ζ�Ŵ���struct kmem_cache����ɹ����˳�
			break;
	}
	BUG_ON(!cache_cache.num);
	cache_cache.gfporder = order;//gfporder��ʾ��slab����2^gfporder��ҳ��
	cache_cache.colour = left_over / cache_cache.colour_off;//��ɫ���Ĵ�С����colour_offΪ��λ
	cache_cache.slab_size = ALIGN(cache_cache.num * sizeof(kmem_bufctl_t) +
				      sizeof(struct slab), cache_line_size());//slab�������Ĵ�С

	/* 2+3) create the kmalloc caches */
	sizes = malloc_sizes;//malloc_sizes�����С
	names = cache_names;//cache_names����cache��

	/*
	 * Initialize the caches that provide memory for the array cache and the
	 * kmem_list3 structures first.  Without this, further allocations will
	 * bug.
	 */
	//���ȴ���struct array_cache��struct kmem_list3���õ�general cache�������Ǻ�����ʼ�������Ļ���
	sizes[INDEX_AC].cs_cachep = kmem_cache_create(names[INDEX_AC].name,
				sizes[INDEX_AC].cs_size,
				ARCH_KMALLOC_MINALIGN,
				ARCH_KMALLOC_FLAGS|SLAB_PANIC,
				NULL);//INDEX_AC�Ǽ���local cache���õ�struct arraycache_init������kmalloc size�е�������
   //��������һ�����С��general cache�������˴�С�����cacheΪlocal cache����

	if (INDEX_AC != INDEX_L3) {//���struct kmem_list3��struct arraycache_init��Ӧ��kmalloc size������ͬ��
   //����С���ڲ�ͬ�ļ����򴴽�struct kmem_list3���õ�cache��������һ��cache
		sizes[INDEX_L3].cs_cachep =
			kmem_cache_create(names[INDEX_L3].name,
				sizes[INDEX_L3].cs_size,
				ARCH_KMALLOC_MINALIGN,
				ARCH_KMALLOC_FLAGS|SLAB_PANIC,
				NULL);
	}

	slab_early_init = 0;//��������������general cache��slab early init�׶ν������ڴ�֮ǰ��������������ʽslab

	while (sizes->cs_size != ULONG_MAX) {//ѭ������kmalloc�������general cache
		/*
		 * For performance, all the general caches are L1 aligned.
		 * This should be particularly beneficial on SMP boxes, as it
		 * eliminates "false sharing".
		 * Note for systems short on memory removing the alignment will
		 * allow tighter packing of the smaller caches.
		 */
		if (!sizes->cs_cachep) {
       //ĳ�����kmalloc cache��δ����������֮��struct kmem_list3��struct arraycache_init��Ӧ��cache�Ѿ���������
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
	}//���ˣ�kmalloc general cache�Ѿ�������ϣ���������ʹ����
	/* 4) Replace the bootstrap head arrays */
	{//���Ĳ�����kmalloc�����滻��̬�����ȫ�ֱ�������ĿǰΪֹһ��ʹ��������ȫ��local cache��һ����cache_cache��
 	//local cacheָ��initarray_cache.cache����һ����malloc_sizes[INDEX_AC].cs_cachep��local cacheָ��
 	//initarray_generic.cache���μ�setup_cpu_cache�����������滻���ǡ�
		struct array_cache *ptr;

		ptr = kmalloc(sizeof(struct arraycache_init), GFP_KERNEL);//����cache_cache����local cache�Ŀռ�

		local_irq_disable();
		BUG_ON(cpu_cache_get(&cache_cache) != &initarray_cache.cache);
		memcpy(ptr, cpu_cache_get(&cache_cache),
		       sizeof(struct arraycache_init));//����ԭcache_cache��local cache����initarray_cache�����µ�λ��
		/*
		 * Do not assume that spinlocks can be initialized via memcpy:
		 */
		spin_lock_init(&ptr->lock);

		cache_cache.array[smp_processor_id()] = ptr;//cache_cache��local cacheָ���µ�λ��
		local_irq_enable();

		ptr = kmalloc(sizeof(struct arraycache_init), GFP_KERNEL);
       //����malloc_sizes[INDEX_AC].cs_cachep����local cache�Ŀռ�

		local_irq_disable();
		BUG_ON(cpu_cache_get(malloc_sizes[INDEX_AC].cs_cachep)
		       != &initarray_generic.cache);
		memcpy(ptr, cpu_cache_get(malloc_sizes[INDEX_AC].cs_cachep),
		       sizeof(struct arraycache_init));//����ԭlocal cache���·����λ�ã�ע���ʱlocal cache�Ĵ�С�ǹ̶���
		/*
		 * Do not assume that spinlocks can be initialized via memcpy:
		 */
		spin_lock_init(&ptr->lock);

		malloc_sizes[INDEX_AC].cs_cachep->array[smp_processor_id()] =
		    ptr;
		local_irq_enable();
	}
	/* 5) Replace the bootstrap kmem_list3's */
	{//���岽������Ĳ����ƣ���kmalloc�Ŀռ��滻��̬�����slab����
		int nid;

		/* Replace the static kmem_list3 structures for the boot cpu */
		init_list(&cache_cache, &initkmem_list3[CACHE_CACHE], node);//����struct kmem_cache��slab����

		for_each_online_node(nid) {
			init_list(malloc_sizes[INDEX_AC].cs_cachep,
				  &initkmem_list3[SIZE_AC + nid], nid);//����struct arraycache_init��slab����

			if (INDEX_AC != INDEX_L3) {
				init_list(malloc_sizes[INDEX_L3].cs_cachep,
					  &initkmem_list3[SIZE_L3 + nid], nid);//����struct kmem_list3��slab����
			}
		}
	}

	/* 6) resize the head arrays to their final sizes */
	{//��ʼ���׶�local cache�Ĵ�С�ǹ̶��ģ�Ҫ���ݶ����С���¼���
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
	g_cpucache_up = FULL;//�󹦸�ɣ�general cache����ȫ������������

	/*
	 * Register a cpu startup notifier callback that initializes
	 * cpu_cache_get for all new cpus
	 */
	register_cpu_notifier(&cpucache_notifier);//ע��cpu up�ص�������cpu upʱ����local cache

	/*
	 * The reap timers are started later, with a module init call: That part
	 * of the kernel is not yet operational.
	 */
} 