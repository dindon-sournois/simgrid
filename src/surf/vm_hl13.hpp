/* Copyright (c) 2013-2014. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include "host_clm03.hpp"
#include "vm_interface.hpp"

#ifndef SURF_VM_HPP_
#define SURF_VM_HPP_

#define GUESTOS_NOISE 100 // This value corresponds to the cost of the global action associated to the VM
                          // It corresponds to the cost of a VM running no tasks.

/***********
 * Classes *
 ***********/

class VMHL13Model;
typedef VMHL13Model *VMHL13ModelPtr;

class VMHL13;
typedef VMHL13 *VMHL13Ptr;

/*********
 * Model *
 *********/
class VMHL13Model : public VMModel {
public:
  VMHL13Model();
  ~VMHL13Model(){};

  VMPtr createVM(const char *name, surf_resource_t host_PM);
  double shareResources(double now);
  void adjustWeightOfDummyCpuActions() {};
  ActionPtr communicate(HostPtr src, HostPtr dst, double size, double rate);
  ActionPtr executeParallelTask(int host_nb,
                                          void **host_list,
                                          double *flops_amount,
                                          double *bytes_amount,
                                          double rate);
  void updateActionsState(double /*now*/, double /*delta*/);
};

/************
 * Resource *
 ************/

class VMHL13 : public VM {
public:
  VMHL13(VMModelPtr model, const char* name, xbt_dict_t props, surf_resource_t host_PM);
  ~VMHL13();

  void suspend();
  void resume();

  void save();
  void restore();

  void migrate(surf_resource_t ind_dst_pm);

  e_surf_resource_state_t getState();
  void setState(e_surf_resource_state_t state);

  surf_resource_t getPm(); // will be vm_ws_get_pm()

  void setBound(double bound);
  void setAffinity(CpuPtr cpu, unsigned long mask);

  //FIXME: remove
  void updateState(tmgr_trace_event_t event_type, double value, double date);
  bool isUsed();

  ActionPtr execute(double size);
  ActionPtr sleep(double duration);
};

/**********
 * Action *
 **********/

#endif /* SURF_VM_HPP_ */