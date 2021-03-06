if(enable_smpi AND enable_smpi_MPICH3_testsuite)
  if(WIN32)
    set(CMAKE_C_FLAGS "-include ${CMAKE_HOME_DIRECTORY}/include/smpi/smpi_main.h")
  else()
    set(CMAKE_C_COMPILER "${CMAKE_BINARY_DIR}/smpi_script/bin/smpicc")
    set(CMAKE_Fortran_COMPILER "${CMAKE_BINARY_DIR}/smpi_script/bin/smpiff")
  endif()

  set(MPICH_FLAGS "-DHAVE_STDLIB_H=1 -DHAVE_UNISTD_H=1 -DHAVE_STRING_H=1 -DUSE_STDARG=1 -DHAVE_LONG_DOUBLE=1 -DHAVE_PROTOTYPES=1 -DHAVE_SIGNAL_H=1 -DHAVE_SIGACTION=1 -DHAVE_SLEEP=1 -DHAVE_SYSCONF=1  -Wno-error=unused-variable")
  include_directories(BEFORE "${CMAKE_HOME_DIRECTORY}/include/smpi")
  include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../include/")

  foreach(file accfence1 accfence2_am accfence2 accpscw1 allocmem epochtest getfence1 getgroup manyrma3 nullpscw
          putfence1 putfidx putpscw1 test1_am test1 test1_dt test2_am test2 test3 test5_am test5 transpose1 transpose2
          transpose3 transpose6 transpose7 window_creation)
    # not compiled files: acc-loc adlb_mimic1 attrorderwin badrma baseattrwin compare_and_swap contention_put
    # contention_putget contig_displ fence_shm fetchandadd_am fetchandadd fetchandadd_tree_am fetchandadd_tree 
    # fetch_and_op fkeyvalwin flush get_acc_local get_accumulate ircpi linked_list_bench_lock_all 
    # linked_list_bench_lock_excl linked_list_bench_lock_shr linked_list linked_list_fop linked_list_lockall 
    # lockcontention2 lockcontention3 lockcontention locknull lockopts manyrma2 mcs-mutex mixedsync mutex_bench 
    # pscw_ordering put_base put_bottom req_example reqops rmanull rmazero rma-contig selfrma strided_acc_indexed 
    # strided_acc_onelock strided_acc_subarray strided_getacc_indexed strided_getacc_indexed_shared strided_get_indexed
    # strided_putget_indexed strided_putget_indexed_shared test3_am test4_am test4 transpose4 transpose5 wincall 
    # win_dynamic_acc win_flavors win_info winname win_shared win_shared_noncontig win_shared_noncontig_put 
    # win_large_shm win_zero wintest
    add_executable(${file} ${file}.c)
    target_link_libraries(${file} simgrid mtest_c)
    set_target_properties(${file} PROPERTIES COMPILE_FLAGS "${MPICH_FLAGS}")
  endforeach()
endif()

if (enable_smpi_MPICH3_testsuite AND HAVE_RAW_CONTEXTS)
  ADD_TEST(test-smpi-mpich3-rma-raw       ${CMAKE_COMMAND} -E chdir ${CMAKE_BINARY_DIR}/teshsuite/smpi/mpich3-test/rma ${PERL_EXECUTABLE} ${CMAKE_HOME_DIRECTORY}/teshsuite/smpi/mpich3-test/runtests ${TESH_OPTION} -mpiexec=${CMAKE_BINARY_DIR}/smpi_script/bin/smpirun -srcdir=${CMAKE_HOME_DIRECTORY}/teshsuite/smpi/mpich3-test/rma -tests=testlist -execarg=--cfg=contexts/factory:raw -execarg=--cfg=smpi/privatize-global-variables:${HAVE_PRIVATIZATION})
  SET_TESTS_PROPERTIES(test-smpi-mpich3-rma-raw PROPERTIES PASS_REGULAR_EXPRESSION "tests passed!")
endif()

foreach(file accfence1 accfence2_am accfence2 accpscw1 allocmem epochtest getfence1 getgroup manyrma3 nullpscw
          putfence1 putfidx putpscw1 test1_am test1 test1_dt test2_am test2 test3 test5_am test5 transpose1 transpose2
          transpose3 transpose6 transpose7 window_creation
          acc-loc adlb_mimic1 attrorderwin badrma baseattrwin compare_and_swap contention_put contention_putget 
          contig_displ fence_shm fetchandadd_am fetchandadd fetchandadd_tree_am fetchandadd_tree fetch_and_op
          fkeyvalwin flush get_acc_local get_accumulate ircpi linked_list_bench_lock_all linked_list_bench_lock_excl
          linked_list_bench_lock_shr linked_list linked_list_fop linked_list_lockall lockcontention2 lockcontention3 
          lockcontention locknull lockopts manyrma2 mcs-mutex mixedsync mutex_bench pscw_ordering put_base put_bottom 
          req_example reqops rmanull rmazero rma-contig selfrma strided_acc_indexed strided_acc_onelock 
          strided_acc_subarray strided_getacc_indexed strided_getacc_indexed_shared strided_get_indexed 
          strided_putget_indexed strided_putget_indexed_shared test3_am test4_am test4 transpose4 transpose5 wincall 
          win_dynamic_acc win_flavors win_info winname win_shared win_shared_noncontig win_shared_noncontig_put 
          win_large_shm win_zero wintest)
  set(examples_src  ${examples_src} ${CMAKE_CURRENT_SOURCE_DIR}/${file}.c)
endforeach()

set(examples_src  ${examples_src}                                           PARENT_SCOPE)
set(txt_files     ${txt_files}     ${CMAKE_CURRENT_SOURCE_DIR}/testlist  
                                   ${CMAKE_CURRENT_SOURCE_DIR}/mcs-mutex.h
                                   ${CMAKE_CURRENT_SOURCE_DIR}/squelch.h    PARENT_SCOPE)
