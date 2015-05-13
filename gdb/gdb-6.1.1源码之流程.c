/***********************************************
 * gdb.c
 ***********************************************/
main()
{
	struct captured_main_args args = {
		argc;
		argv;
		use_windows = 0;
		interpreter_p = "console";
	};
	return gdb_main(&args);
}
/***********************************************
 * main.c
 ***********************************************/
gdb_main(struct captured_main_args* args)
{
	catch_errors(captured_main, args, "", RETURN_MASK_ALL=(RETURN_MASK_QUIT | RETURN_MASK_ERROR));	//总之做了一些事之后就执行captured_main
}
/***********************************************
 * top.c
 ***********************************************/
catch_errors(catch_errors_ftype *func=captured_main,
				void *func_args=前一args,
				char *errstring="",
				return_mask mask=(RETURN_MASK_QUIT | RETURN_MASK_ERROR))
{
	int val;
	enum return_reason caught;	//意外返回的原因，通常不返回
	struct catch_errors_args args = {
		func = captured_main;
		func_args = 前一args;
	};
	catcher(do_catch_errors, uiout, &args, &val, &caught, errstring="", NULL, mask);
	/*
	 *	uiout -> {
			flag = 0;
			impl -> 实现全部默认
			其它未初始化
		}
	 */
	...
}
/***********************************************
 * top.c
 ***********************************************/
static void
catcher(catch_exceptions_ftype *func=do_catch_errors,
		struct ui_out *func_uiout=uiout,
		void *func_args=args,
		int *func_val,
		enum return_reason *func_caught,
		char *errstring="",
		char **gdberrmsg=NULL,
		return_mask mask=(RETURN_MASK_QUIT | RETURN_MASK_ERROR))
{
	...
	if (!caught)
		val = (*func)(func_uiout, func_args) = do_catch_errors(uiout, args);
}
/***********************************************
 * top.c
 ***********************************************/
static int
do_catch_errors(struct ui_out *uiout, void *data)
{
  struct catch_errors_args *args = data;
  return args->func (args->func_args) = captured_main(前一args={argc; argv; use_windows = 0; interpreter_p = "console";});
}
/***********************************************
 * main.c
 ***********************************************/
static int captured_main(void *data)
{
	...
	gdb_init (argv[0]);	//初始化所有文件
	...
	while (1)
	{
		catch_errors (captured_command_loop, 0, "", RETURN_MASK_ALL);	//循环执行captured_command_loop()
	}
}
/***********************************************
 * main.c
 ***********************************************/
static int captured_command_loop (void *data=0)
{
	current_interp_command_loop();
	do_cleanups (ALL_CLEANUPS=(struct cleanup *)0);
	quit_command (NULL, instream == stdin);
	return 1;
}
/***********************************************
 * Interps.c
 ***********************************************/
void current_interp_command_loop(void)
{
	...
	cli_command_loop();
}
/***********************************************
 * event-top.c
 ***********************************************/
void cli_command_loop(void)
{
	...
	start_event_loop();
}
/***********************************************
 * event-top.c
 ***********************************************/
void start_event_loop(void)
{
	while (1)
	{
		int gdb_result;
		
		gdb_result = catch_errors (gdb_do_one_event, 0, "", RETURN_MASK_ALL);
		if (gdb_result < 0)
			break;
		if (gdb_result == 0)
		{
			display_gdb_prompt (0);
			if (after_char_processing_hook)
				(*after_char_processing_hook) ();
		}
	}
}
/***********************************************
 * event-loop.c
 ***********************************************/
int gdb_do_one_event(void *data)
{
	if (process_event())
	{
		return 1;
	}
	poll_timers();
	if (gdb_wait_for_event() < 0)
	{
		return -1;
	}
	if (process_event())
	{
		return 1;
	}
	return 1;
}
/***********************************************
 * event-loop.c
 ***********************************************/
static int gdb_wait_for_event(void)
{
	file_handler *file_ptr;
	gdb_event *file_event_ptr;
	int num_found = 0;
	int i;
	if (use_poll)
    {
      num_found =
	poll (gdb_notifier.poll_fds,
	      (unsigned long) gdb_notifier.num_fds,
	      gdb_notifier.timeout_valid ? gdb_notifier.poll_timeout : -1);
	}
	
	if (use_poll)
	{
		for (i = 0; (i < gdb_notifier.num_fds) && (num_found > 0); i++)
		{
			if ((gdb_notifier.poll_fds + i)->revents)
				num_found--;
			else
				continue;

			for (file_ptr = gdb_notifier.first_file_handler;
				file_ptr != NULL;
			file_ptr = file_ptr->next_file)
			{
				if (file_ptr->fd == (gdb_notifier.poll_fds + i)->fd)
				break;
			}
			if (file_ptr)
			{
				if (file_ptr->ready_mask == 0)
				{
					file_event_ptr = create_file_event (file_ptr->fd);
					async_queue_event (file_event_ptr, TAIL);	//从尾部插入事件
				}
			}
			file_ptr->ready_mask = (gdb_notifier.poll_fds + i)->revents;
		}
	}
}