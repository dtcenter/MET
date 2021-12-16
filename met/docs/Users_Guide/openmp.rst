.. _openmp:

OpenMP
======

Introduction
____________

There are a number of different ways of parallelizing code. OpenMP offers parallelism within a single shared-memory workstation or supercomputer node.  The programmer writes OpenMP directives into the code to parallelize particular code regions.

When a parallelized code region is reached, which we shall hereafter call a parallel region, a number of threads are spawned and work is shared among them. Running on different cores, this reduces the execution time. At the end of the parallel region, the code returns to single-thread execution.

A limited number of code regions are parallelized in MET. As a consequence, there are limits to the overall speed gains achievable. Only the parallel regions of code will get faster with more threads, leaving the remaining serial portions to dominate the runtime. 

Not all top-level executables use parallelized code.  If OpenMP is available, a log message will appear inviting the user to increase the number of threads for faster runtimes.

Setting the number of threads
_____________________________

The number of threads is controlled by the environment variable `OMP_NUM_THREADS`. For example, on a quad core machine, the user might choose to run on 4 threads:

.. code :: bash

    export OMP_NUM_THREADS=4

Alternatively, the variable may be specified as a prefix to the executable itself. For example:

.. code :: bash

    OMP_NUM_THREADS=4 <exec>

The case where this variable remains unset is handled inside the code, which defaults to a single thread.

There are choices when deciding how many threads to use. To perform a single run as fast as possible, it would likely be appropriate to use as many threads as there are (physical) cores available on the specific system.  However, it is not a cast-iron guarantee that more threads will always translate into more speed. In theory, there is a chance that running across multiple NUMA regions may carry negative performance impacts. This has not been observed in practice, however. 

A lower thread count is appropriate when time-to-solution is not so critical, because cores remain idle when the code is not inside a parallel region.  Fewer threads typically means better resource utilization.

Which code is parallelized?
___________________________

Regions of parallelized code are:

  * :code:`fractional_coverage (data_plane_util.cc)`

Only the following top-level executables can presently benefit from OpenMP parallelization:

  * :code:`grid_stat`
  * :code:`ensemble_stat`
  * :code:`grid_ens_prod`

Thread binding
______________

It is normally beneficial to bind threads to particular cores, sometimes called *affinitization*. There are a few reasons for this, but at the very least it guarantees that threads remain evenly distributed across the available cores. Otherwise, the operating system may migrate threads between cores during a run.

OpenMP provides some environment variables to handle this: :code:`OMP_PLACES` and :code:`OMP_PROC_BIND`.  We anticipate that the effect of setting only :code:`OMP_PROC_BIND=true` would be neutral-to-positive.

However, there are sometimes compiler-specific environment variables. Instead, thread affinitization is sometimes handled by MPI launchers, since OpenMP is often used in MPI codes to reduce intra-node communications.

Where code is running in a production context, it is worth being familiar with the binding / affinitization method on the particular system and building it into any relevant scripting.


