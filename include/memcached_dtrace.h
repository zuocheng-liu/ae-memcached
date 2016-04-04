/*
 * Generated by dtrace(1M).
 */

#ifndef _MEMCACHED_DTRACE_H
#define _MEMCACHED_DTRACE_H


#ifdef __cplusplus
extern "C" {
#endif

#if ENABLE_DTRACE

#define MEMCACHED_ASSOC_DELETE(arg0, arg1) \
 __dtrace_memcached___assoc__delete(arg0, arg1)
#define MEMCACHED_ASSOC_DELETE_ENABLED() \
 __dtraceenabled_memcached___assoc__delete()
#define MEMCACHED_ASSOC_FIND(arg0, arg1) \
 __dtrace_memcached___assoc__find(arg0, arg1)
#define MEMCACHED_ASSOC_FIND_ENABLED() \
 __dtraceenabled_memcached___assoc__find()
#define MEMCACHED_ASSOC_INSERT(arg0, arg1) \
 __dtrace_memcached___assoc__insert(arg0, arg1)
#define MEMCACHED_ASSOC_INSERT_ENABLED() \
 __dtraceenabled_memcached___assoc__insert()
#define MEMCACHED_COMMAND_ADD(arg0, arg1, arg2) \
 __dtrace_memcached___command__add(arg0, arg1, arg2)
#define MEMCACHED_COMMAND_ADD_ENABLED() \
 __dtraceenabled_memcached___command__add()
#define MEMCACHED_COMMAND_APPEND(arg0, arg1, arg2) \
 __dtrace_memcached___command__append(arg0, arg1, arg2)
#define MEMCACHED_COMMAND_APPEND_ENABLED() \
 __dtraceenabled_memcached___command__append()
#define MEMCACHED_COMMAND_CAS(arg0, arg1, arg2, arg3) \
 __dtrace_memcached___command__cas(arg0, arg1, arg2, arg3)
#define MEMCACHED_COMMAND_CAS_ENABLED() \
 __dtraceenabled_memcached___command__cas()
#define MEMCACHED_COMMAND_DECR(arg0, arg1, arg2) \
 __dtrace_memcached___command__decr(arg0, arg1, arg2)
#define MEMCACHED_COMMAND_DECR_ENABLED() \
 __dtraceenabled_memcached___command__decr()
#define MEMCACHED_COMMAND_DELETE(arg0, arg1, arg2) \
 __dtrace_memcached___command__delete(arg0, arg1, arg2)
#define MEMCACHED_COMMAND_DELETE_ENABLED() \
 __dtraceenabled_memcached___command__delete()
#define MEMCACHED_COMMAND_GET(arg0, arg1, arg2) \
 __dtrace_memcached___command__get(arg0, arg1, arg2)
#define MEMCACHED_COMMAND_GET_ENABLED() \
 __dtraceenabled_memcached___command__get()
#define MEMCACHED_COMMAND_GETS(arg0, arg1, arg2, arg3) \
 __dtrace_memcached___command__gets(arg0, arg1, arg2, arg3)
#define MEMCACHED_COMMAND_GETS_ENABLED() \
 __dtraceenabled_memcached___command__gets()
#define MEMCACHED_COMMAND_INCR(arg0, arg1, arg2) \
 __dtrace_memcached___command__incr(arg0, arg1, arg2)
#define MEMCACHED_COMMAND_INCR_ENABLED() \
 __dtraceenabled_memcached___command__incr()
#define MEMCACHED_COMMAND_PREPEND(arg0, arg1, arg2) \
 __dtrace_memcached___command__prepend(arg0, arg1, arg2)
#define MEMCACHED_COMMAND_PREPEND_ENABLED() \
 __dtraceenabled_memcached___command__prepend()
#define MEMCACHED_COMMAND_REPLACE(arg0, arg1, arg2) \
 __dtrace_memcached___command__replace(arg0, arg1, arg2)
#define MEMCACHED_COMMAND_REPLACE_ENABLED() \
 __dtraceenabled_memcached___command__replace()
#define MEMCACHED_COMMAND_SET(arg0, arg1, arg2) \
 __dtrace_memcached___command__set(arg0, arg1, arg2)
#define MEMCACHED_COMMAND_SET_ENABLED() \
 __dtraceenabled_memcached___command__set()
#define MEMCACHED_CONN_ALLOCATE(arg0) \
 __dtrace_memcached___conn__allocate(arg0)
#define MEMCACHED_CONN_ALLOCATE_ENABLED() \
 __dtraceenabled_memcached___conn__allocate()
#define MEMCACHED_CONN_CREATE(arg0) \
 __dtrace_memcached___conn__create(arg0)
#define MEMCACHED_CONN_CREATE_ENABLED() \
 __dtraceenabled_memcached___conn__create()
#define MEMCACHED_CONN_DESTROY(arg0) \
 __dtrace_memcached___conn__destroy(arg0)
#define MEMCACHED_CONN_DESTROY_ENABLED() \
 __dtraceenabled_memcached___conn__destroy()
#define MEMCACHED_CONN_DISPATCH(arg0, arg1) \
 __dtrace_memcached___conn__dispatch(arg0, arg1)
#define MEMCACHED_CONN_DISPATCH_ENABLED() \
 __dtraceenabled_memcached___conn__dispatch()
#define MEMCACHED_CONN_RELEASE(arg0) \
 __dtrace_memcached___conn__release(arg0)
#define MEMCACHED_CONN_RELEASE_ENABLED() \
 __dtraceenabled_memcached___conn__release()
#define MEMCACHED_ITEM_LINK(arg0, arg1) \
 __dtrace_memcached___item__link(arg0, arg1)
#define MEMCACHED_ITEM_LINK_ENABLED() \
 __dtraceenabled_memcached___item__link()
#define MEMCACHED_ITEM_REMOVE(arg0, arg1) \
 __dtrace_memcached___item__remove(arg0, arg1)
#define MEMCACHED_ITEM_REMOVE_ENABLED() \
 __dtraceenabled_memcached___item__remove()
#define MEMCACHED_ITEM_REPLACE(arg0, arg1, arg2, arg3) \
 __dtrace_memcached___item__replace(arg0, arg1, arg2, arg3)
#define MEMCACHED_ITEM_REPLACE_ENABLED() \
 __dtraceenabled_memcached___item__replace()
#define MEMCACHED_ITEM_UNLINK(arg0, arg1) \
 __dtrace_memcached___item__unlink(arg0, arg1)
#define MEMCACHED_ITEM_UNLINK_ENABLED() \
 __dtraceenabled_memcached___item__unlink()
#define MEMCACHED_ITEM_UPDATE(arg0, arg1) \
 __dtrace_memcached___item__update(arg0, arg1)
#define MEMCACHED_ITEM_UPDATE_ENABLED() \
 __dtraceenabled_memcached___item__update()
#define MEMCACHED_PROCESS_COMMAND_END(arg0, arg1, arg2) \
 __dtrace_memcached___process__command__end(arg0, arg1, arg2)
#define MEMCACHED_PROCESS_COMMAND_END_ENABLED() \
 __dtraceenabled_memcached___process__command__end()
#define MEMCACHED_PROCESS_COMMAND_START(arg0, arg1, arg2) \
 __dtrace_memcached___process__command__start(arg0, arg1, arg2)
#define MEMCACHED_PROCESS_COMMAND_START_ENABLED() \
 __dtraceenabled_memcached___process__command__start()
#define MEMCACHED_SLABS_ALLOCATE(arg0, arg1, arg2, arg3) \
 __dtrace_memcached___slabs__allocate(arg0, arg1, arg2, arg3)
#define MEMCACHED_SLABS_ALLOCATE_ENABLED() \
 __dtraceenabled_memcached___slabs__allocate()
#define MEMCACHED_SLABS_ALLOCATE_FAILED(arg0, arg1) \
 __dtrace_memcached___slabs__allocate__failed(arg0, arg1)
#define MEMCACHED_SLABS_ALLOCATE_FAILED_ENABLED() \
 __dtraceenabled_memcached___slabs__allocate__failed()
#define MEMCACHED_SLABS_FREE(arg0, arg1, arg2) \
 __dtrace_memcached___slabs__free(arg0, arg1, arg2)
#define MEMCACHED_SLABS_FREE_ENABLED() \
 __dtraceenabled_memcached___slabs__free()
#define MEMCACHED_SLABS_SLABCLASS_ALLOCATE(arg0) \
 __dtrace_memcached___slabs__slabclass__allocate(arg0)
#define MEMCACHED_SLABS_SLABCLASS_ALLOCATE_ENABLED() \
 __dtraceenabled_memcached___slabs__slabclass__allocate()
#define MEMCACHED_SLABS_SLABCLASS_ALLOCATE_FAILED(arg0) \
 __dtrace_memcached___slabs__slabclass__allocate__failed(arg0)
#define MEMCACHED_SLABS_SLABCLASS_ALLOCATE_FAILED_ENABLED() \
 __dtraceenabled_memcached___slabs__slabclass__allocate__failed()


extern void __dtrace_memcached___assoc__delete(char *, int);
extern int __dtraceenabled_memcached___assoc__delete(void);
extern void __dtrace_memcached___assoc__find(char *, int);
extern int __dtraceenabled_memcached___assoc__find(void);
extern void __dtrace_memcached___assoc__insert(char *, int);
extern int __dtraceenabled_memcached___assoc__insert(void);
extern void __dtrace_memcached___command__add(int, char *, int);
extern int __dtraceenabled_memcached___command__add(void);
extern void __dtrace_memcached___command__append(int, char *, int);
extern int __dtraceenabled_memcached___command__append(void);
extern void __dtrace_memcached___command__cas(int, char *, int, int64_t);
extern int __dtraceenabled_memcached___command__cas(void);
extern void __dtrace_memcached___command__decr(int, char *, int64_t);
extern int __dtraceenabled_memcached___command__decr(void);
extern void __dtrace_memcached___command__delete(int, char *, long);
extern int __dtraceenabled_memcached___command__delete(void);
extern void __dtrace_memcached___command__get(int, char *, int);
extern int __dtraceenabled_memcached___command__get(void);
extern void __dtrace_memcached___command__gets(int, char *, int, int64_t);
extern int __dtraceenabled_memcached___command__gets(void);
extern void __dtrace_memcached___command__incr(int, char *, int64_t);
extern int __dtraceenabled_memcached___command__incr(void);
extern void __dtrace_memcached___command__prepend(int, char *, int);
extern int __dtraceenabled_memcached___command__prepend(void);
extern void __dtrace_memcached___command__replace(int, char *, int);
extern int __dtraceenabled_memcached___command__replace(void);
extern void __dtrace_memcached___command__set(int, char *, int);
extern int __dtraceenabled_memcached___command__set(void);
extern void __dtrace_memcached___conn__allocate(int);
extern int __dtraceenabled_memcached___conn__allocate(void);
extern void __dtrace_memcached___conn__create(void *);
extern int __dtraceenabled_memcached___conn__create(void);
extern void __dtrace_memcached___conn__destroy(void *);
extern int __dtraceenabled_memcached___conn__destroy(void);
extern void __dtrace_memcached___conn__dispatch(int, int);
extern int __dtraceenabled_memcached___conn__dispatch(void);
extern void __dtrace_memcached___conn__release(int);
extern int __dtraceenabled_memcached___conn__release(void);
extern void __dtrace_memcached___item__link(char *, int);
extern int __dtraceenabled_memcached___item__link(void);
extern void __dtrace_memcached___item__remove(char *, int);
extern int __dtraceenabled_memcached___item__remove(void);
extern void __dtrace_memcached___item__replace(char *, int, char *, int);
extern int __dtraceenabled_memcached___item__replace(void);
extern void __dtrace_memcached___item__unlink(char *, int);
extern int __dtraceenabled_memcached___item__unlink(void);
extern void __dtrace_memcached___item__update(char *, int);
extern int __dtraceenabled_memcached___item__update(void);
extern void __dtrace_memcached___process__command__end(int, void *, int);
extern int __dtraceenabled_memcached___process__command__end(void);
extern void __dtrace_memcached___process__command__start(int, void *, int);
extern int __dtraceenabled_memcached___process__command__start(void);
extern void __dtrace_memcached___slabs__allocate(int, int, int, void *);
extern int __dtraceenabled_memcached___slabs__allocate(void);
extern void __dtrace_memcached___slabs__allocate__failed(int, int);
extern int __dtraceenabled_memcached___slabs__allocate__failed(void);
extern void __dtrace_memcached___slabs__free(int, int, void *);
extern int __dtraceenabled_memcached___slabs__free(void);
extern void __dtrace_memcached___slabs__slabclass__allocate(int);
extern int __dtraceenabled_memcached___slabs__slabclass__allocate(void);
extern void __dtrace_memcached___slabs__slabclass__allocate__failed(int);
extern int __dtraceenabled_memcached___slabs__slabclass__allocate__failed(void);

#else

#define MEMCACHED_ASSOC_DELETE(arg0, arg1)
#define MEMCACHED_ASSOC_DELETE_ENABLED() (0)
#define MEMCACHED_ASSOC_FIND(arg0, arg1)
#define MEMCACHED_ASSOC_FIND_ENABLED() (0)
#define MEMCACHED_ASSOC_INSERT(arg0, arg1)
#define MEMCACHED_ASSOC_INSERT_ENABLED() (0)
#define MEMCACHED_COMMAND_ADD(arg0, arg1, arg2)
#define MEMCACHED_COMMAND_ADD_ENABLED() (0)
#define MEMCACHED_COMMAND_APPEND(arg0, arg1, arg2)
#define MEMCACHED_COMMAND_APPEND_ENABLED() (0)
#define MEMCACHED_COMMAND_CAS(arg0, arg1, arg2, arg3)
#define MEMCACHED_COMMAND_CAS_ENABLED() (0)
#define MEMCACHED_COMMAND_DECR(arg0, arg1, arg2)
#define MEMCACHED_COMMAND_DECR_ENABLED() (0)
#define MEMCACHED_COMMAND_DELETE(arg0, arg1, arg2)
#define MEMCACHED_COMMAND_DELETE_ENABLED() (0)
#define MEMCACHED_COMMAND_GET(arg0, arg1, arg2)
#define MEMCACHED_COMMAND_GET_ENABLED() (0)
#define MEMCACHED_COMMAND_GETS(arg0, arg1, arg2, arg3)
#define MEMCACHED_COMMAND_GETS_ENABLED() (0)
#define MEMCACHED_COMMAND_INCR(arg0, arg1, arg2)
#define MEMCACHED_COMMAND_INCR_ENABLED() (0)
#define MEMCACHED_COMMAND_PREPEND(arg0, arg1, arg2)
#define MEMCACHED_COMMAND_PREPEND_ENABLED() (0)
#define MEMCACHED_COMMAND_REPLACE(arg0, arg1, arg2)
#define MEMCACHED_COMMAND_REPLACE_ENABLED() (0)
#define MEMCACHED_COMMAND_SET(arg0, arg1, arg2)
#define MEMCACHED_COMMAND_SET_ENABLED() (0)
#define MEMCACHED_CONN_ALLOCATE(arg0)
#define MEMCACHED_CONN_ALLOCATE_ENABLED() (0)
#define MEMCACHED_CONN_CREATE(arg0)
#define MEMCACHED_CONN_CREATE_ENABLED() (0)
#define MEMCACHED_CONN_DESTROY(arg0)
#define MEMCACHED_CONN_DESTROY_ENABLED() (0)
#define MEMCACHED_CONN_DISPATCH(arg0, arg1)
#define MEMCACHED_CONN_DISPATCH_ENABLED() (0)
#define MEMCACHED_CONN_RELEASE(arg0)
#define MEMCACHED_CONN_RELEASE_ENABLED() (0)
#define MEMCACHED_ITEM_LINK(arg0, arg1)
#define MEMCACHED_ITEM_LINK_ENABLED() (0)
#define MEMCACHED_ITEM_REMOVE(arg0, arg1)
#define MEMCACHED_ITEM_REMOVE_ENABLED() (0)
#define MEMCACHED_ITEM_REPLACE(arg0, arg1, arg2, arg3)
#define MEMCACHED_ITEM_REPLACE_ENABLED() (0)
#define MEMCACHED_ITEM_UNLINK(arg0, arg1)
#define MEMCACHED_ITEM_UNLINK_ENABLED() (0)
#define MEMCACHED_ITEM_UPDATE(arg0, arg1)
#define MEMCACHED_ITEM_UPDATE_ENABLED() (0)
#define MEMCACHED_PROCESS_COMMAND_END(arg0, arg1, arg2)
#define MEMCACHED_PROCESS_COMMAND_END_ENABLED() (0)
#define MEMCACHED_PROCESS_COMMAND_START(arg0, arg1, arg2)
#define MEMCACHED_PROCESS_COMMAND_START_ENABLED() (0)
#define MEMCACHED_SLABS_ALLOCATE(arg0, arg1, arg2, arg3)
#define MEMCACHED_SLABS_ALLOCATE_ENABLED() (0)
#define MEMCACHED_SLABS_ALLOCATE_FAILED(arg0, arg1)
#define MEMCACHED_SLABS_ALLOCATE_FAILED_ENABLED() (0)
#define MEMCACHED_SLABS_FREE(arg0, arg1, arg2)
#define MEMCACHED_SLABS_FREE_ENABLED() (0)
#define MEMCACHED_SLABS_SLABCLASS_ALLOCATE(arg0)
#define MEMCACHED_SLABS_SLABCLASS_ALLOCATE_ENABLED() (0)
#define MEMCACHED_SLABS_SLABCLASS_ALLOCATE_FAILED(arg0)
#define MEMCACHED_SLABS_SLABCLASS_ALLOCATE_FAILED_ENABLED() (0)

#endif


#ifdef __cplusplus
}
#endif

#endif /* _MEMCACHED_DTRACE_H */
