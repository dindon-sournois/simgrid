/* Copyright (c) 2013-2014. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

/*****************************************************************************

 * Function: alltoall_bruck

 * Return: int

 * inputs:
    send_buff: send input buffer
    send_count: number of elements to send
    send_type: data type of elements being sent
    recv_buff: receive output buffer
    recv_count: number of elements to received
    recv_type: data type of elements being received
    comm: communicator

 * Descrp: Function realizes the alltoall operation using the bruck algorithm.

 * Auther: MPICH / modified by Ahmad Faraj

 ****************************************************************************/
int
smpi_coll_tuned_alltoall_bruck(void *send_buff, int send_count,
                               MPI_Datatype send_type, void *recv_buff,
                               int recv_count, MPI_Datatype recv_type,
                               MPI_Comm comm)
{
  MPI_Status status;
  MPI_Aint extent;
  MPI_Datatype new_type;

  int *blocks_length, *disps;
  int i, src, dst, rank, num_procs, count, remainder, block, position;
  int pack_size, tag = COLL_TAG_ALLTOALL, pof2 = 1;


  char *tmp_buff;
  char *send_ptr = (char *) send_buff;
  char *recv_ptr = (char *) recv_buff;

  num_procs = smpi_comm_size(comm);
  rank = smpi_comm_rank(comm);

  extent = smpi_datatype_get_extent(recv_type);

  tmp_buff = (char *) smpi_get_tmp_sendbuffer(num_procs * recv_count * extent);
  disps = (int *) xbt_malloc(sizeof(int) * num_procs);
  blocks_length = (int *) xbt_malloc(sizeof(int) * num_procs);

  smpi_mpi_sendrecv(send_ptr + rank * send_count * extent,
               (num_procs - rank) * send_count, send_type, rank, tag,
               recv_ptr, (num_procs - rank) * recv_count, recv_type, rank,
               tag, comm, &status);

  smpi_mpi_sendrecv(send_ptr, rank * send_count, send_type, rank, tag,
               recv_ptr + (num_procs - rank) * recv_count * extent,
               rank * recv_count, recv_type, rank, tag, comm, &status);



  MPI_Pack_size(send_count * num_procs, send_type, comm, &pack_size);

  while (pof2 < num_procs) {
    dst = (rank + pof2) % num_procs;
    src = (rank - pof2 + num_procs) % num_procs;


    count = 0;
    for (block = 1; block < num_procs; block++)
      if (block & pof2) {
        blocks_length[count] = send_count;
        disps[count] = block * send_count;
        count++;
      }

    MPI_Type_indexed(count, blocks_length, disps, recv_type, &new_type);
    smpi_datatype_commit(&new_type);

    position = 0;
    MPI_Pack(recv_buff, 1, new_type, tmp_buff, pack_size, &position, comm);

    smpi_mpi_sendrecv(tmp_buff, position, MPI_PACKED, dst, tag, recv_buff, 1,
                 new_type, src, tag, comm, &status);
    smpi_datatype_unuse(new_type);

    pof2 *= 2;
  }

  free(disps);
  free(blocks_length);

  smpi_mpi_sendrecv(recv_ptr + (rank + 1) * recv_count * extent,
               (num_procs - rank - 1) * recv_count, send_type,
               rank, tag, tmp_buff, (num_procs - rank - 1) * recv_count,
               recv_type, rank, tag, comm, &status);

  smpi_mpi_sendrecv(recv_ptr, (rank + 1) * recv_count, send_type, rank, tag,
               tmp_buff + (num_procs - rank - 1) * recv_count * extent,
               (rank + 1) * recv_count, recv_type, rank, tag, comm, &status);


  for (i = 0; i < num_procs; i++)
    smpi_mpi_sendrecv(tmp_buff + i * recv_count * extent, recv_count, send_type,
                 rank, tag,
                 recv_ptr + (num_procs - i - 1) * recv_count * extent,
                 recv_count, recv_type, rank, tag, comm, &status);

  smpi_free_tmp_buffer(tmp_buff);
  return MPI_SUCCESS;
}
