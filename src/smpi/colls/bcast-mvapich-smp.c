/* Copyright (c) 2013-2014. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

/*
 * Copyright (c) 2004-2005 The Trustees of Indiana University and Indiana
 *                         University Research and Technology
 *                         Corporation.  All rights reserved.
 * Copyright (c) 2004-2009 The University of Tennessee and The University
 *                         of Tennessee Research Foundation.  All rights
 *                         reserved.
 * Copyright (c) 2004-2005 High Performance Computing Center Stuttgart,
 *                         University of Stuttgart.  All rights reserved.
 * Copyright (c) 2004-2005 The Regents of the University of California.
 *                         All rights reserved.
 *
 * Additional copyrights may follow
 */
 /* -*- Mode: C; c-basic-offset:4 ; -*- */
/* Copyright (c) 2001-2014, The Ohio State University. All rights
 * reserved.
 *
 * This file is part of the MVAPICH2 software package developed by the
 * team members of The Ohio State University's Network-Based Computing
 * Laboratory (NBCL), headed by Professor Dhabaleswar K. (DK) Panda.
 *
 * For detailed copyright and licensing information, please refer to the
 * copyright file COPYRIGHT in the top level MVAPICH2 directory.
 */
/*
 *
 *  (C) 2001 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */
#include "colls_private.h"


extern int (*MV2_Bcast_function) (void *buffer, int count, MPI_Datatype datatype,
                           int root, MPI_Comm comm_ptr);

extern int (*MV2_Bcast_intra_node_function) (void *buffer, int count, MPI_Datatype datatype,
                                      int root, MPI_Comm comm_ptr);
                                      
extern int zcpy_knomial_factor;
extern int mv2_pipelined_zcpy_knomial_factor;
extern int bcast_segment_size;
extern int mv2_inter_node_knomial_factor;
extern int mv2_intra_node_knomial_factor;
extern int mv2_bcast_two_level_system_size;
#define INTRA_NODE_ROOT 0

#define MPIR_Pipelined_Bcast_Zcpy_MV2 smpi_coll_tuned_bcast_mpich
#define MPIR_Pipelined_Bcast_MV2 smpi_coll_tuned_bcast_mpich
#define MPIR_Bcast_binomial_MV2 smpi_coll_tuned_bcast_binomial_tree
#define MPIR_Bcast_scatter_ring_allgather_shm_MV2 smpi_coll_tuned_bcast_scatter_LR_allgather
#define MPIR_Bcast_scatter_doubling_allgather_MV2 smpi_coll_tuned_bcast_scatter_rdb_allgather
#define MPIR_Bcast_scatter_ring_allgather_MV2 smpi_coll_tuned_bcast_scatter_LR_allgather
#define MPIR_Shmem_Bcast_MV2 smpi_coll_tuned_bcast_mpich
#define MPIR_Bcast_tune_inter_node_helper_MV2 smpi_coll_tuned_bcast_mvapich2_inter_node
#define MPIR_Bcast_inter_node_helper_MV2 smpi_coll_tuned_bcast_mvapich2_inter_node
#define MPIR_Knomial_Bcast_intra_node_MV2 smpi_coll_tuned_bcast_mvapich2_knomial_intra_node
#define MPIR_Bcast_intra_MV2 smpi_coll_tuned_bcast_mvapich2_intra_node

extern int zcpy_knomial_factor;
extern int mv2_pipelined_zcpy_knomial_factor;
extern int bcast_segment_size;
extern int mv2_inter_node_knomial_factor;
extern int mv2_intra_node_knomial_factor;
#define mv2_bcast_two_level_system_size  64
#define mv2_bcast_short_msg             16384
#define mv2_bcast_large_msg            512*1024
#define mv2_knomial_intra_node_threshold 131072
#define mv2_scatter_rd_inter_leader_bcast 1
int smpi_coll_tuned_bcast_mvapich2_inter_node(void *buffer,
                                                 int count,
                                                 MPI_Datatype datatype,
                                                 int root,
                                                 MPI_Comm  comm)
{
    int rank;
    int mpi_errno = MPI_SUCCESS;
    MPI_Comm shmem_comm, leader_comm;
    int local_rank, local_size, global_rank = -1;
    int leader_root, leader_of_root;


    rank = smpi_comm_rank(comm);
    //comm_size = smpi_comm_size(comm);


    if (MV2_Bcast_function==NULL){
      MV2_Bcast_function=smpi_coll_tuned_bcast_mpich;
    }
    
    if (MV2_Bcast_intra_node_function==NULL){
      MV2_Bcast_intra_node_function= smpi_coll_tuned_bcast_mpich;
    }
    
    if(smpi_comm_get_leaders_comm(comm)==MPI_COMM_NULL){
      smpi_comm_init_smp(comm);
    }
    
    shmem_comm = smpi_comm_get_intra_comm(comm);
    local_rank = smpi_comm_rank(shmem_comm);
    local_size = smpi_comm_size(shmem_comm);

    leader_comm = smpi_comm_get_leaders_comm(comm);

    if ((local_rank == 0) && (local_size > 1)) {
      global_rank = smpi_comm_rank(leader_comm);
    }

    int* leaders_map = smpi_comm_get_leaders_map(comm);
    leader_of_root = smpi_group_rank(smpi_comm_group(comm),leaders_map[root]);
    leader_root = smpi_group_rank(smpi_comm_group(leader_comm),leaders_map[root]);
    
    
    if (local_size > 1) {
        if ((local_rank == 0) && (root != rank) && (leader_root == global_rank)) {
            smpi_mpi_recv(buffer, count, datatype, root,
                                     COLL_TAG_BCAST, comm, MPI_STATUS_IGNORE);
        }
        if ((local_rank != 0) && (root == rank)) {
            smpi_mpi_send(buffer, count, datatype,
                                     leader_of_root, COLL_TAG_BCAST, comm);
        }
    }
#if defined(_MCST_SUPPORT_)
    if (comm_ptr->ch.is_mcast_ok) {
        mpi_errno = MPIR_Mcast_inter_node_MV2(buffer, count, datatype, root, comm_ptr,
                                              errflag);
        if (mpi_errno == MPI_SUCCESS) {
            goto fn_exit;
        } else {
            goto fn_fail;
        }
    }
#endif
/*
    if (local_rank == 0) {
        leader_comm = smpi_comm_get_leaders_comm(comm);
        root = leader_root;
    }

    if (MV2_Bcast_function == &MPIR_Pipelined_Bcast_MV2) {
        mpi_errno = MPIR_Pipelined_Bcast_MV2(buffer, count, datatype,
                                             root, comm);
    } else if (MV2_Bcast_function == &MPIR_Bcast_scatter_ring_allgather_shm_MV2) {
        mpi_errno = MPIR_Bcast_scatter_ring_allgather_shm_MV2(buffer, count,
                                                              datatype, root,
                                                              comm);
    } else */{
        if (local_rank == 0) {
      /*      if (MV2_Bcast_function == &MPIR_Knomial_Bcast_inter_node_wrapper_MV2) {
                mpi_errno = MPIR_Knomial_Bcast_inter_node_wrapper_MV2(buffer, count,
                                                              datatype, root,
                                                              comm);
            } else {*/
                mpi_errno = MV2_Bcast_function(buffer, count, datatype,
                                               leader_root, leader_comm);
          //  }
        }
    }

    return mpi_errno;
}


int smpi_coll_tuned_bcast_mvapich2_knomial_intra_node(void *buffer,
                                      int count,
                                      MPI_Datatype datatype,
                                      int root, MPI_Comm  comm)
{
    int local_size = 0, rank;
    int mpi_errno = MPI_SUCCESS;
    MPI_Request *reqarray = NULL;
    MPI_Status *starray = NULL;
    int src, dst, mask, relative_rank;
    int k;
    if (MV2_Bcast_function==NULL){
      MV2_Bcast_function=smpi_coll_tuned_bcast_mpich;
    }
    
    if (MV2_Bcast_intra_node_function==NULL){
      MV2_Bcast_intra_node_function= smpi_coll_tuned_bcast_mpich;
    }
    
    if(smpi_comm_get_leaders_comm(comm)==MPI_COMM_NULL){
      smpi_comm_init_smp(comm);
    }
    
    local_size = smpi_comm_size(comm);
    rank = smpi_comm_rank(comm);


    reqarray=(MPI_Request *)xbt_malloc(2 * mv2_intra_node_knomial_factor * sizeof (MPI_Request));

    starray=(MPI_Status *)xbt_malloc(2 * mv2_intra_node_knomial_factor * sizeof (MPI_Status));

    /* intra-node k-nomial bcast  */
    if (local_size > 1) {
        relative_rank = (rank >= root) ? rank - root : rank - root + local_size;
        mask = 0x1;

        while (mask < local_size) {
            if (relative_rank % (mv2_intra_node_knomial_factor * mask)) {
                src = relative_rank / (mv2_intra_node_knomial_factor * mask) *
                    (mv2_intra_node_knomial_factor * mask) + root;
                if (src >= local_size) {
                    src -= local_size;
                }

                smpi_mpi_recv(buffer, count, datatype, src,
                                         COLL_TAG_BCAST, comm,
                                         MPI_STATUS_IGNORE);
                break;
            }
            mask *= mv2_intra_node_knomial_factor;
        }
        mask /= mv2_intra_node_knomial_factor;

        while (mask > 0) {
            int reqs = 0;
            for (k = 1; k < mv2_intra_node_knomial_factor; k++) {
                if (relative_rank + mask * k < local_size) {
                    dst = rank + mask * k;
                    if (dst >= local_size) {
                        dst -= local_size;
                    }
                    reqarray[reqs++]=smpi_mpi_isend(buffer, count, datatype, dst,
                                              COLL_TAG_BCAST, comm);
                }
            }
            smpi_mpi_waitall(reqs, reqarray, starray);

            mask /= mv2_intra_node_knomial_factor;
        }
    }
    xbt_free(reqarray);
    xbt_free(starray);
    return mpi_errno;
}


int smpi_coll_tuned_bcast_mvapich2_intra_node(void *buffer,
                         int count,
                         MPI_Datatype datatype,
                         int root, MPI_Comm  comm)
{
    int mpi_errno = MPI_SUCCESS;
    int comm_size;
    int two_level_bcast = 1;
    size_t nbytes = 0; 
    int is_homogeneous, is_contig;
    MPI_Aint type_size;
    void *tmp_buf = NULL;
    MPI_Comm shmem_comm;

    if (count == 0)
        return MPI_SUCCESS;
    if (MV2_Bcast_function==NULL){
      MV2_Bcast_function=smpi_coll_tuned_bcast_mpich;
    }
    
    if (MV2_Bcast_intra_node_function==NULL){
      MV2_Bcast_intra_node_function= smpi_coll_tuned_bcast_mpich;
    }
    
    if(smpi_comm_get_leaders_comm(comm)==MPI_COMM_NULL){
      smpi_comm_init_smp(comm);
    }
    
    comm_size = smpi_comm_size(comm);
   // rank = smpi_comm_rank(comm);
/*
    if (HANDLE_GET_KIND(datatype) == HANDLE_KIND_BUILTIN)*/
        is_contig = 1;
/*    else {
        MPID_Datatype_get_ptr(datatype, dtp);
        is_contig = dtp->is_contig;
    }
*/
    is_homogeneous = 1;
#ifdef MPID_HAS_HETERO
    if (comm_ptr->is_hetero)
        is_homogeneous = 0;
#endif

    /* MPI_Type_size() might not give the accurate size of the packed
     * datatype for heterogeneous systems (because of padding, encoding,
     * etc). On the other hand, MPI_Pack_size() can become very
     * expensive, depending on the implementation, especially for
     * heterogeneous systems. We want to use MPI_Type_size() wherever
     * possible, and MPI_Pack_size() in other places.
     */
    //if (is_homogeneous) {
        type_size=smpi_datatype_size(datatype);
    //}
/*    else {*/
/*        MPIR_Pack_size_impl(1, datatype, &type_size);*/
/*    }*/
    nbytes = (size_t) (count) * (type_size);
    if (comm_size <= mv2_bcast_two_level_system_size) {
        if (nbytes > mv2_bcast_short_msg && nbytes < mv2_bcast_large_msg) {
            two_level_bcast = 1;
        } else {
            two_level_bcast = 0;
        }
    }

    if (two_level_bcast == 1
#if defined(_MCST_SUPPORT_)
            || comm_ptr->ch.is_mcast_ok
#endif
        ) {

        if (!is_contig || !is_homogeneous) {
            tmp_buf=(void *)smpi_get_tmp_sendbuffer(nbytes);

            /* TODO: Pipeline the packing and communication */
           // position = 0;
/*            if (rank == root) {*/
/*                mpi_errno =*/
/*                    MPIR_Pack_impl(buffer, count, datatype, tmp_buf, nbytes, &position);*/
/*                if (mpi_errno)*/
/*                    MPIU_ERR_POP(mpi_errno);*/
/*            }*/
        }

        shmem_comm = smpi_comm_get_intra_comm(comm);
        if (!is_contig || !is_homogeneous) {
            mpi_errno =
                MPIR_Bcast_inter_node_helper_MV2(tmp_buf, nbytes, MPI_BYTE,
                                                 root, comm);
        } else {
            mpi_errno =
                MPIR_Bcast_inter_node_helper_MV2(buffer, count, datatype, root,
                                                 comm);
        }

        /* We are now done with the inter-node phase */
            if (nbytes <= mv2_knomial_intra_node_threshold) {
                if (!is_contig || !is_homogeneous) {
                    mpi_errno = MPIR_Shmem_Bcast_MV2(tmp_buf, nbytes, MPI_BYTE,
                                                     root, shmem_comm);
                } else {
                    mpi_errno = MPIR_Shmem_Bcast_MV2(buffer, count, datatype,
                                                     root, shmem_comm);
                }
            } else {
                if (!is_contig || !is_homogeneous) {
                    mpi_errno =
                        MPIR_Knomial_Bcast_intra_node_MV2(tmp_buf, nbytes,
                                                          MPI_BYTE,
                                                          INTRA_NODE_ROOT,
                                                          shmem_comm);
                } else {
                    mpi_errno =
                        MPIR_Knomial_Bcast_intra_node_MV2(buffer, count,
                                                          datatype,
                                                          INTRA_NODE_ROOT,
                                                          shmem_comm);
                }
            }

    } else {
        if (nbytes <= mv2_bcast_short_msg) {
            mpi_errno = MPIR_Bcast_binomial_MV2(buffer, count, datatype, root,
                                                comm);
        } else {
            if (mv2_scatter_rd_inter_leader_bcast) {
                mpi_errno = MPIR_Bcast_scatter_ring_allgather_MV2(buffer, count,
                                                                  datatype,
                                                                  root,
                                                                  comm);
            } else {
                mpi_errno =
                    MPIR_Bcast_scatter_doubling_allgather_MV2(buffer, count,
                                                              datatype, root,
                                                              comm);
            }
        }
    }


    return mpi_errno;

}
