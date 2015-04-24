
struct rcu_node;  //同步相关

struct task_struct {
	volatile long state;	/* -1 unrunnable, 0 runnable, >0 stopped */	//状态
	void *stack;
	atomic_t usage;
	unsigned int flags;	/* per process flags, defined below */
	unsigned int ptrace;

	int lock_depth;		/* BKL lock depth */

#ifdef CONFIG_SMP
#ifdef __ARCH_WANT_UNLOCKED_CTXSW
	int oncpu;
#endif
#endif
//调度信息
	int prio, static_prio, normal_prio;  //优先级
	unsigned int rt_priority;
	const struct sched_class *sched_class;
	struct sched_entity se;
	struct sched_rt_entity rt;

#ifdef CONFIG_PREEMPT_NOTIFIERS
	/* list of struct preempt_notifier: */
	struct hlist_head preempt_notifiers;
#endif

	/*
	 * fpu_counter contains the number of consecutive context switches
	 * that the FPU is used. If this is over a threshold, the lazy fpu
	 * saving becomes unlazy to save the trap. This is an unsigned char
	 * so that after 256 times the counter wraps and the behavior turns
	 * lazy again; this to deal with bursty apps that only use FPU for
	 * a short time
	 */
	unsigned char fpu_counter;
#ifdef CONFIG_BLK_DEV_IO_TRACE
	unsigned int btrace_seq;
#endif

	unsigned int policy;
	cpumask_t cpus_allowed;

#ifdef CONFIG_TREE_PREEMPT_RCU
	int rcu_read_lock_nesting;
	char rcu_read_unlock_special;
	struct rcu_node *rcu_blocked_node;
	struct list_head rcu_node_entry;
#endif /* #ifdef CONFIG_TREE_PREEMPT_RCU */

#if defined(CONFIG_SCHEDSTATS) || defined(CONFIG_TASK_DELAY_ACCT)
	struct sched_info sched_info;
#endif

	struct list_head tasks;
	struct plist_node pushable_tasks;

	struct mm_struct *mm, *active_mm;

/* task state */
	int exit_state;
	int exit_code, exit_signal;
	int pdeath_signal;  /*  The signal sent when the parent dies  */
	/* ??? */
	unsigned int personality;
	unsigned did_exec:1;
	unsigned in_execve:1;	/* Tell the LSMs that the process is doing an
				 * execve */
	unsigned in_iowait:1;


	/* Revert to default priority/policy when forking */
	unsigned sched_reset_on_fork:1;

	pid_t pid;
	pid_t tgid;

#ifdef CONFIG_CC_STACKPROTECTOR
	/* Canary value for the -fstack-protector gcc feature */
	unsigned long stack_canary;
#endif

	/* 
	 * pointers to (original) parent process, youngest child, younger sibling,
	 * older sibling, respectively.  (p->father can be replaced with 
	 * p->real_parent->pid)
	 */
	struct task_struct *real_parent; /* real parent process */
	struct task_struct *parent; /* recipient of SIGCHLD, wait4() reports */
	/*
	 * children/sibling forms the list of my natural children
	 */
	struct list_head children;	/* list of my children */
	struct list_head sibling;	/* linkage in my parent's children list */
	struct task_struct *group_leader;	/* threadgroup leader */

	/*
	 * ptraced is the list of tasks this task is using ptrace on.
	 * This includes both natural children and PTRACE_ATTACH targets.
	 * p->ptrace_entry is p's link on the p->parent->ptraced list.
	 */
	struct list_head ptraced;
	struct list_head ptrace_entry;

	/*
	 * This is the tracer handle for the ptrace BTS extension.
	 * This field actually belongs to the ptracer task.
	 */
	struct bts_context *bts;

	/* PID/PID hash table linkage. */
	struct pid_link pids[PIDTYPE_MAX];
	struct list_head thread_group;

	struct completion *vfork_done;		/* for vfork() */
	int __user *set_child_tid;		/* CLONE_CHILD_SETTID */
	int __user *clear_child_tid;		/* CLONE_CHILD_CLEARTID */

	cputime_t utime, stime, utimescaled, stimescaled;
	cputime_t gtime;
	cputime_t prev_utime, prev_stime;
	unsigned long nvcsw, nivcsw; /* context switch counts */
	struct timespec start_time; 		/* monotonic time */
	struct timespec real_start_time;	/* boot based time */
/* mm fault and swap info: this can arguably be seen as either mm-specific or thread-specific */
	unsigned long min_flt, maj_flt;

	struct task_cputime cputime_expires;
	struct list_head cpu_timers[3];

/* process credentials */
	const struct cred *real_cred;	/* objective and real subjective task
					 * credentials (COW) */
	const struct cred *cred;	/* effective (overridable) subjective task
					 * credentials (COW) */
	struct mutex cred_guard_mutex;	/* guard against foreign influences on
					 * credential calculations
					 * (notably. ptrace) */
	struct cred *replacement_session_keyring; /* for KEYCTL_SESSION_TO_PARENT */

	char comm[TASK_COMM_LEN]; /* executable name excluding path
				     - access with [gs]et_task_comm (which lock
				       it with task_lock())
				     - initialized normally by flush_old_exec */
/* file system info */
	int link_count, total_link_count;
#ifdef CONFIG_SYSVIPC
/* ipc stuff */
	struct sysv_sem sysvsem;
#endif
#ifdef CONFIG_DETECT_HUNG_TASK
/* hung task detection */
	unsigned long last_switch_count;
#endif
/* CPU-specific state of this task */
	struct thread_struct thread;
/* filesystem information */
	struct fs_struct *fs;
/* open file information */
	struct files_struct *files;
/* namespaces */
	struct nsproxy *nsproxy;
/* signal handlers */
	struct signal_struct *signal;	//信号相关
	struct sighand_struct *sighand;

	sigset_t blocked, real_blocked;
	sigset_t saved_sigmask;	/* restored if set_restore_sigmask() was used */
	struct sigpending pending;	//信号相关

	unsigned long sas_ss_sp;  //信号处理函数备用堆栈地址，sa->sigaction(?)
	size_t sas_ss_size;			//由于历史原因，irqaction描述符的标志前缀也是SA_
	int (*notifier)(void *priv);
	void *notifier_data;
	sigset_t *notifier_mask;
	struct audit_context *audit_context;
#ifdef CONFIG_AUDITSYSCALL
	uid_t loginuid;
	unsigned int sessionid;
#endif
	seccomp_t seccomp;

/* Thread group tracking */
   	u32 parent_exec_id;
   	u32 self_exec_id;
/* Protection of (de-)allocation: mm, files, fs, tty, keyrings, mems_allowed,
 * mempolicy */
	spinlock_t alloc_lock;

#ifdef CONFIG_GENERIC_HARDIRQS
	/* IRQ handler threads */
	struct irqaction *irqaction;
#endif

	/* Protection of the PI data structures: */
	spinlock_t pi_lock;

#ifdef CONFIG_RT_MUTEXES
	/* PI waiters blocked on a rt_mutex held by this task */
	struct plist_head pi_waiters;
	/* Deadlock detection and priority inheritance handling */
	struct rt_mutex_waiter *pi_blocked_on;
#endif

#ifdef CONFIG_DEBUG_MUTEXES
	/* mutex deadlock detection */
	struct mutex_waiter *blocked_on;
#endif
#ifdef CONFIG_TRACE_IRQFLAGS
	unsigned int irq_events;
	int hardirqs_enabled;
	unsigned long hardirq_enable_ip;
	unsigned int hardirq_enable_event;
	unsigned long hardirq_disable_ip;
	unsigned int hardirq_disable_event;
	int softirqs_enabled;
	unsigned long softirq_disable_ip;
	unsigned int softirq_disable_event;
	unsigned long softirq_enable_ip;
	unsigned int softirq_enable_event;
	int hardirq_context;
	int softirq_context;
#endif
#ifdef CONFIG_LOCKDEP
# define MAX_LOCK_DEPTH 48UL
	u64 curr_chain_key;
	int lockdep_depth;
	unsigned int lockdep_recursion;
	struct held_lock held_locks[MAX_LOCK_DEPTH];
	gfp_t lockdep_reclaim_gfp;
#endif

/* journalling filesystem info */
	void *journal_info;

/* stacked block device info */
	struct bio *bio_list, **bio_tail;

/* VM state */
	struct reclaim_state *reclaim_state;

	struct backing_dev_info *backing_dev_info;

	struct io_context *io_context;

	unsigned long ptrace_message;
	siginfo_t *last_siginfo; /* For ptrace use.  */
	struct task_io_accounting ioac;
#if defined(CONFIG_TASK_XACCT)
	u64 acct_rss_mem1;	/* accumulated rss usage */
	u64 acct_vm_mem1;	/* accumulated virtual memory usage */
	cputime_t acct_timexpd;	/* stime + utime since last update */
#endif
#ifdef CONFIG_CPUSETS
	nodemask_t mems_allowed;	/* Protected by alloc_lock */
	int cpuset_mem_spread_rotor;
#endif
#ifdef CONFIG_CGROUPS
	/* Control Group info protected by css_set_lock */
	struct css_set *cgroups;
	/* cg_list protected by css_set_lock and tsk->alloc_lock */
	struct list_head cg_list;
#endif
#ifdef CONFIG_FUTEX
	struct robust_list_head __user *robust_list;
#ifdef CONFIG_COMPAT
	struct compat_robust_list_head __user *compat_robust_list;
#endif
	struct list_head pi_state_list;
	struct futex_pi_state *pi_state_cache;
#endif
#ifdef CONFIG_PERF_EVENTS
	struct perf_event_context *perf_event_ctxp;
	struct mutex perf_event_mutex;
	struct list_head perf_event_list;
#endif
#ifdef CONFIG_NUMA
	struct mempolicy *mempolicy;	/* Protected by alloc_lock */
	short il_next;
#endif
	atomic_t fs_excl;	/* holding fs exclusive resources */
	struct rcu_head rcu;

	/*
	 * cache last used pipe for splice
	 */
	struct pipe_inode_info *splice_pipe;
#ifdef	CONFIG_TASK_DELAY_ACCT
	struct task_delay_info *delays;
#endif
#ifdef CONFIG_FAULT_INJECTION
	int make_it_fail;
#endif
	struct prop_local_single dirties;
#ifdef CONFIG_LATENCYTOP
	int latency_record_count;
	struct latency_record latency_record[LT_SAVECOUNT];
#endif
	/*
	 * time slack values; these are used to round up poll() and
	 * select() etc timeout values. These are in nanoseconds.
	 */
	unsigned long timer_slack_ns;
	unsigned long default_timer_slack_ns;

	struct list_head	*scm_work_list;
#ifdef CONFIG_FUNCTION_GRAPH_TRACER
	/* Index of current stored adress in ret_stack */
	int curr_ret_stack;
	/* Stack of return addresses for return function tracing */
	struct ftrace_ret_stack	*ret_stack;
	/* time stamp for last schedule */
	unsigned long long ftrace_timestamp;
	/*
	 * Number of functions that haven't been traced
	 * because of depth overrun.
	 */
	atomic_t trace_overrun;
	/* Pause for the tracing */
	atomic_t tracing_graph_pause;
#endif
#ifdef CONFIG_TRACING
	/* state flags for use by tracers */
	unsigned long trace;
	/* bitmask of trace recursion */
	unsigned long trace_recursion;
#endif /* CONFIG_TRACING */
	unsigned long stack_start;
};
