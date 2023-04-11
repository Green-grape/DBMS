Project information
==================
> ##### This is expanded version of project3. In project 3, we can only access DBMS with single thread. But this project can access DBMS with multi thread (only for db_update() & db_find()) with using lock table of project4.

Handling Concurrency Control
================
> ###### Concurrency control is really good method for improving user experience and performance of DBMS. But we can confront with deadlock problem. This project solves this problem with strict 2PL & wait-for-graph. Detail explanation of this project is followings.

Lock Mode
===============
**We use two lock modes shared(read) & exclusive(write)**. The reason of deviding lock mode is that shared(read) mode doesn't affect to data of DBMS, but exclusive(write) affects to data of DBMS. Like meaning of each words. 

Shared mode can share lock with other thread in same record. In my design, it looks like each thread has lock, but only head lock of record has lock and other decribe lock with flag(flag=1: acquired, flag=0: waiting.)

Exclusive mode only one thread can get lock in one record. So, infront lock of exclusive lock is shared mode, then current lock go to sleep. Except all thread of front lock same to current lock.

Deadlock Detection
=================
> ##### In deadlock detection, my design checks that "If this lock is acquired to target record, it can be deadlock?". For checking above information, we need to 2 kinds of deadlock. 

1) same thread tries to acquire lock of same record.
2) Mutual waiting in different record.

In 1), first check existence of requested lock. If requested lock or greater authority lock(acquired: Exclusive, requested: Shared) exists and has acquired status, it doesn't dead lock. Second(it doesn't satisfy first condition), check all lock of in record is acquired by same thread. If it is true, it doesn't dead lock. Else it is dead lock!

In 2), first get all lock of record. Second, get trx from lock and search trx that it is waiting for "trying to acquire thread". If it is ture, it is dead lock. Else it isn't.

Abort and Rollback
==================
Abort is done by above deadlock detection. If abort is run, rollback all data of buffer with buf_abort_done(). 
**In this time, I don't write to disk**. Instead, set isDirty to 1 and rewrite to disk at eviction time. After all rollback process is done, release all lock of trx. 

project 5 API
=================
1. int trx_begin(void)
get trx_id(>=1) and allocate trx with trx_id.
return trx_id.(error: return 0)

2. int trx_commit(int trx_id)
delete buffer saved data for Rollback and release all lock in trx,
delete trx.
return trx_id.(error: return 0)

3. int db_find(int table_id, int64_t key, char* ret_val, int trx_id)
find value with table_id & key.
return 0: success
return -1: cannot find data
return -2: abort!

4. int db_update(int table_id, int64_t key, char* values, int trx_id)
update value with find data with table_id & key.
return 0: success
return -1: cannot find data
return -2: abort!






