#include <cassert>
#include <stdio.h>
#include <memory>
#include <vector>
#include <iostream>
#include <thread>
#include <libkahypar.h>
static kahypar_context_t *context = NULL;


#if __has_include("disabled-libmtkahypar.h")
#define HAS_MT 1
#include <libmtkahypar.h>
static mt_kahypar_context_t *mtcontext = NULL;
#else
#define HAS_MT 0
#endif
#include <mtkahypar.h>

#define LDBG 0

static mt_kahypar_context_t *mt_context = NULL;
extern "C" {
int mtkahypar_test_partitioner(char *filename) {
    mt_kahypar_error_t error{};

  printf("In mtkahypar_initialize call\n");
  // return 0;

  if (1) {
    mt_kahypar_initialize(
      std::thread::hardware_concurrency(),
      true);
    printf("Hardware concurrency %d\n", std::thread::hardware_concurrency());
  }

  printf("Called an MT function\n");

  // Setup partitioning context
  // mt_kahypar_context_t* context = mt_kahypar_context_from_preset(DEFAULT);
  mt_kahypar_context_t* context = mt_kahypar_context_from_file("/usr/local/etc/mtkahypar.ini", &error);
  // In the following, we partition a hypergraph into two blocks
  // with an allowed imbalance of 3% and optimize the connective metric (KM1)
  mt_kahypar_set_partitioning_parameters(context,
    2 /* number of blocks */, 0.03 /* imbalance parameter */,
    KM1 /* objective function */);
  mt_kahypar_set_seed(42 /* seed */);
  // Enable logging
  mt_kahypar_status_t status =
    mt_kahypar_set_context_parameter(context, VERBOSE, "1", &error);
  assert(status == SUCCESS);

  // Load Hypergraph for DEFAULT preset
  mt_kahypar_hypergraph_t hypergraph =
    mt_kahypar_read_hypergraph_from_file(filename,
      context, HMETIS /* file format */, &error);
  if (hypergraph.hypergraph == nullptr) {
    std::cout << error.msg << std::endl; std::exit(1);
  }

  // Partition Hypergraph
  mt_kahypar_partitioned_hypergraph_t partitioned_hg =
    mt_kahypar_partition(hypergraph, context, &error);
  if (partitioned_hg.partitioned_hg == nullptr) {
    std::cout << error.msg << std::endl; std::exit(1);
  }

  // Extract Partition
  auto partition = std::make_unique<mt_kahypar_partition_id_t[]>(
    mt_kahypar_num_hypernodes(hypergraph));
  mt_kahypar_get_partition(partitioned_hg, partition.get());

  // Extract Block Weights
  auto block_weights = std::make_unique<mt_kahypar_hypernode_weight_t[]>(2);
  mt_kahypar_get_block_weights(partitioned_hg, block_weights.get());

  // Compute Metrics
  const double imbalance = mt_kahypar_imbalance(partitioned_hg, context);
  const int km1 = mt_kahypar_km1(partitioned_hg);

  // Output Results
  std::cout << "Partitioning Results:" << std::endl;
  std::cout << "Imbalance         = " << imbalance << std::endl;
  std::cout << "Km1               = " << km1 << std::endl;
  std::cout << "Weight of Block 0 = " << block_weights[0] << std::endl;
  std::cout << "Weight of Block 1 = " << block_weights[1] << std::endl;

  mt_kahypar_free_context(context);
  mt_kahypar_free_hypergraph(hypergraph);
  mt_kahypar_free_partitioned_hypergraph(partitioned_hg);
  return 0;
}
}

extern "C"
{
  void mtkahypar_hello()
  {
    printf("Testing the partitioner\n");
    mtkahypar_test_partitioner("benches/small.hgr");
  }

  int mtkahypar_evaluate(int num_edges, size_t *eind, kahypar_hyperedge_id_t *eptr, int *part)
  {
    int total = 0;
    for (int i = 0; i < num_edges; ++i)
    {
      int side0 = 0;
      int side1 = 0;
      for (int j = eind[i]; j < eind[i + 1]; ++j)
      {
        if (part[eptr[j]])
          side1 = 1;
        else
          side0 = 1;
      }
      if (side0 && side1)
        ++total;
    }
    return total;
  }

  void mtkahypar_partition_file(char *filename) {

  }

  void mtkahypar_partition(unsigned int nvtxs, unsigned int nhedges, int *hewt, int *vtw, size_t *eind, mt_kahypar_hyperedge_id_t *eptr, int *part, int kway, int passes, long seed, float imbalance)
  {
    mt_kahypar_error_t error{};
    for (int i = 0; i < nvtxs; ++i)
    {
      // part[i] = -1;
    }
    // printf("In the MT wrapper.  DIRECT CALL TO TEST FUNCTION\n");
    // mtkahypar_test_partitioner();
    // printf("Done with test function.\n");
    // return;

    if (mt_context == NULL) {
      printf("mt-KaHyPar context loading\n");
      //mt_context = mt_kahypar_context_from_file("/usr/local/etc/mtkahypar.ini", &error);
      mt_context = mt_kahypar_context_from_preset(DEFAULT);

      mt_kahypar_initialize(
         std::thread::hardware_concurrency(),
         true);
      printf("mt-KaHyPar hardware concurrency %d\n", std::thread::hardware_concurrency());
    }

    // printf("Got past reading the context\n");
    mt_kahypar_status_t status =
         mt_kahypar_set_context_parameter(mt_context, VERBOSE, "0", &error);
    assert(status == SUCCESS);

    mt_kahypar_set_partitioning_parameters(mt_context, kway, imbalance, CUT);
    // With multi-thread, seems to not get consisted results for any
    // given seed.  Probably race conditions on the threads.
    mt_kahypar_set_seed(seed);

    // printf("Set params and seed\n");

    if (0) {
      printf("%d vertices, %d edges\n", nvtxs, nhedges);
      for (int i = 0; i < 6; ++i)
      {
        printf("EIND %d -- %lu\n", i, eind[i]);
      }
      printf("-----\n");
      for (int i = 0; i < 6; ++i)
      {
        printf("EPTR %d -- %lu\n", i, eptr[i]);
      }    
    }
    
    mt_kahypar_hypergraph_t hypergraph;
    if (1) {
      hypergraph = mt_kahypar_create_hypergraph(mt_context, nvtxs, nhedges,
            eptr, eind,
            hewt, vtw, &error);
      if (hypergraph.hypergraph == NULL) {
        printf("Hypergraph creation error %s", error.msg);
        return;
      }
      // printf("Created the hypergraph\n");
    }
    else {
      printf("Now trying to read from a file - benches/small.hgr\n");
      hypergraph = mt_kahypar_read_hypergraph_from_file("benches/small.hgr", mt_context, HMETIS, &error);
    }

    if (hypergraph.hypergraph == nullptr) {
      std::cout << error.msg << std::endl; std::exit(1);
    }
    // bool checkit = mt_kahypar_check_compatibility(hypergraph, DEFAULT);
    // if (checkit) printf("Checkit is true\n");

    mt_kahypar_add_fixed_vertices(hypergraph, part, kway, &error);

    mt_kahypar_partitioned_hypergraph_t partitioned_hg = mt_kahypar_partition(hypergraph, mt_context, &error);
    // printf("Ran mt_kahypar_partition seed was %d\n", seed);
    mt_kahypar_get_partition(partitioned_hg, part);
    mt_kahypar_free_partitioned_hypergraph(partitioned_hg);
    mt_kahypar_free_hypergraph(hypergraph);

    if (0) {
      for (int i = 0; i < 5; ++i)
      {
        printf("Partition result %d -- %d\n", i, part[i]);
      }
    }
  }
}
