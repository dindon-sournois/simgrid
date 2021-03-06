/* Copyright (c) 2006-2016. The SimGrid Team. All rights reserved.          */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include <string>
#include <vector>

#include <xbt/sysdep.h>

#include <simgrid/s4u.h>

#include "s4u_basic.h"

int main(int argc, char **argv) {
  simgrid::s4u::Engine *e = new simgrid::s4u::Engine(&argc,argv);
  e->loadPlatform("../../platforms/two_hosts.xml");
  e->registerFunction<Worker>("worker");
  e->registerFunction<Master>("master");
  e->loadDeployment(argv[1]);
  e->run();
  return 0;
}
