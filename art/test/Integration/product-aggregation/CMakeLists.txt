art_dictionary(NO_INSTALL)

simple_plugin(EventGenerator       "module" NO_INSTALL)
simple_plugin(ParticleSimulator    "module" NO_INSTALL)
simple_plugin(Reconstruction       "module" NO_INSTALL)
simple_plugin(CheckMoreProducts    "module" NO_INSTALL USE_BOOST_UNIT)
simple_plugin(CheckProducts        "module" NO_INSTALL USE_BOOST_UNIT)
simple_plugin(AssembleProducts     "module" NO_INSTALL)
simple_plugin(AssembleMoreProducts "module" NO_INSTALL USE_BOOST_UNIT)
simple_plugin(AssembleMoreProductsRobust "module" NO_INSTALL USE_BOOST_UNIT)

# For the product aggregation tests, a physics workflow has been
# simulated, using multiple stages (see below for schematic of when
# products are produced).
#
#   Stage 1: EVENT GENERATION - Simulate 200 events, with 50 events
#   per output file.  This stage creates 4 output files.
#
#   Stage 2: PARTICLE SIMULATION - Typically, this would be the Geant
#   stage.  For this test, we simply assign energies to the particles
#   generated in stage 1.  This stage creates 4 output files.
#
#   Stage 3: EVENT RECONSTRUCTION - In this stage, we would simulate
#   detector response, etc.  For simplicity, we require that the event
#   have a particle with energy of 50 (arbitrary units) or higher.
#   This stage creates 8 output files, switching to a new file after
#   20 events have been written to the file.
#
#   --- TEST PATH 1 ---
#
#   Stage 4: OUTPUT-FILE CONCATENATION - We simply combine all of the
#   files from Stage 3 so that the Stage-5 analysis will not require
#   any custom product aggregation.  This stage creates 1 output file,
#   with all Event products dropped.
#
#   Stage 5: PHYSICS ANALYSIS - We fetch the Run and SubRun products
#   and check that their aggregated ranges and values (as
#   automatically performed by the framework) are correct.
#
#   --- TEST PATH 2 ---
#
#   Stage 4: ASSEMBLE PRODUCTS - The purpose of this stage is to test
#   the SummedValue<T> class.  We do this by taking SubRun products:
#       1. From an earlier module of the same process, and
#       2. From an input file.
#   We aggregate the products 'by hand' using the SummedValue<T> class,
#   and then 'put' two new products onto the SubRun once the ranges of
#   the dependent products are consistent.  Since these new products
#   are not 'put' during each begin/endSubRun call, dummy products
#   will be inserted into the SubRuns tree.  Aggregating these dummy
#   products with the one product that is 'put' will be tested in
#   Stage 5. This stage creates 1 output file, with all Event products
#   dropped.
#
#   Stage 5: PHYSICS ANALYSIS - Now, we make the same types of checks
#   we made in PATH 1, Stage 5, except we also check that the new
#   products 'put' in Stage 4 are correctly aggregated--i.e.
#   aggregating the 'put' product with the dummy products should
#   result in the same value as just the 'put' product.  There are no
#   outputs with this stage.`
#
# N.B. This series of tests is fragile in that it relies on consistent
#      random-number generation--i.e. the numbers as produced by a
#      generator for a given distribution are always the same for the
#      same C++ code.  Different compiler implementations may do
#      things differently -- i.e. we might need to adjust this set
#      of tests whenever we adopt clang in addition to gcc.
#
# PATH 1 product schematic
#
# Product      |                           PROCESS NAME
# creation     |
# location     | eventGen        particleSim      reco               concat    checkProducts
# ========================================================================================
# beginRun     |                  Geometry------------------------------------------->
#              |
# beginSubRun  |                                   CalibConstants-------------------->
#              |
# event        | GenParticles-------------------------------------------x (dropped)
#              |                  particleEnergies----------------------x (dropped)
#              |
# endSubRun    | nParticles---------------------------------------------------------->
#              |                                   TrackEfficiency------------------->
#              |
# endRun       | nPOTs--------------------------------------------------------------->
#
#
# PATH 2 product schematic
#
# Product      |                           PROCESS NAME
# creation     |
# location     | eventGen        particleSim      reco               userAssembled    checkProducts
# =================================================================================================
# beginRun     |                  Geometry-------------------------------------------------->
#              |
# beginSubRun  |                                   CalibConstants--------------------------->
#              |                                                     TrkEffValue------------>
#              |
# event        | GenParticles---------------------------------------------x (dropped)
#              |                  particleEnergies------------------------x (dropped)
#              |
# endSubRun    | nParticles----------------------------------------------------------------->
#              |                                   TrackEfficiency-------------------------->
#              |                                                     seenParticles---------->
#              |                                                     ParticleRatio---------->
# endRun       | nPOTs---------------------------------------------------------------------->


cet_test(Stage1_EventGeneration_w HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all -c eventGeneration_w.fcl
  DATAFILES
  fcl/eventGeneration_w.fcl
  )

cet_test(Stage2_ParticleSimulation_w HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all -c particleSimulation_w.fcl
  DATAFILES
  fcl/particleSimulation_w.fcl
  TEST_PROPERTIES
  DEPENDS Stage1_EventGeneration_w
  )

cet_test(Stage3_Reconstruction_w HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all -c reconstruction_w.fcl
  DATAFILES
  fcl/reconstruction_w.fcl
  TEST_PROPERTIES
  DEPENDS Stage2_ParticleSimulation_w
  )

# TEST PATH 1

cet_test(Stage4_Concatenation_w HANDBUILT
  TEST_EXEC art
  TEST_ARGS --rethrow-all -c concatenate_w.fcl
  DATAFILES
  fcl/concatenate_w.fcl
  TEST_PROPERTIES
  DEPENDS Stage3_Reconstruction_w
  )

cet_test(Stage5_Analysis_r HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS
  -- -c checkProducts_r.fcl
  DATAFILES
  fcl/checkProducts_r.fcl
  fcl/trackEfficiency.fcl
  TEST_PROPERTIES
  DEPENDS Stage4_Concatenation_w
  )

# TEST PATH 2

cet_test(Stage4_Assemble_w HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS -- -c assemble_w.fcl
  DATAFILES
  fcl/assemble_w.fcl
  fcl/trackEfficiency.fcl
  TEST_PROPERTIES
  DEPENDS Stage3_Reconstruction_w
  )

cet_test(Stage5_MoreAnalysis_r HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS
  -- -c checkMoreProducts_r.fcl
  DATAFILES
  fcl/checkMoreProducts_r.fcl
  fcl/trackEfficiency.fcl
  TEST_PROPERTIES
  DEPENDS Stage4_Assemble_w
  )

cet_test(Stage4_Assemble_reordered_w HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS -- -c assemble_reordered_files_w.fcl
  DATAFILES
  fcl/assemble_reordered_files_w.fcl
  fcl/assemble_w.fcl
  fcl/trackEfficiency.fcl
  TEST_PROPERTIES
  DEPENDS Stage3_Reconstruction_w
  )

cet_test(Stage5_MoreAnalysis_reordered_r HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS
  -- -c checkMoreProducts_reordered_files_r.fcl
  DATAFILES
  fcl/checkMoreProducts_reordered_files_r.fcl
  fcl/checkMoreProducts_r.fcl
  fcl/trackEfficiency.fcl
  TEST_PROPERTIES
  DEPENDS Stage4_Assemble_reordered_w
  )
