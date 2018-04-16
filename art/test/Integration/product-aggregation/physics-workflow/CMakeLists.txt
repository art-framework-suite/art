# vim: set sw=2 :

art_dictionary(NO_INSTALL)

simple_plugin(EventGenerator       "module" NO_INSTALL)
simple_plugin(ParticleSimulator    "module" NO_INSTALL)
simple_plugin(Reconstruction       "module" NO_INSTALL)
simple_plugin(CheckMoreProducts    "module" NO_INSTALL USE_BOOST_UNIT)
simple_plugin(CheckProducts        "module" NO_INSTALL USE_BOOST_UNIT)
simple_plugin(AssembleProducts     "module" NO_INSTALL)
simple_plugin(AssembleMoreProducts "module" NO_INSTALL USE_BOOST_UNIT)
simple_plugin(AssembleMoreProductsRobust "module" NO_INSTALL USE_BOOST_UNIT)

# For more realistic product aggregation tests, a physics workflow has
# been simulated, using multiple stages (see below for schematic of
# when products are produced).
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

set(NUM_THREADS 1 2 4 8)

cet_test(Stage1_EventGeneration_w HANDBUILT
  TEST_EXEC art
  PARG_J -j ${NUM_THREADS}
  TEST_ARGS --rethrow-all -c eventGeneration_w.fcl
  DATAFILES fcl/eventGeneration_w.fcl
)

cet_test(Stage2_ParticleSimulation_w HANDBUILT
  TEST_EXEC art
  PARG_J -j ${NUM_THREADS}
  TEST_ARGS --rethrow-all -c particleSimulation_w.fcl
    -s ../Stage1_EventGeneration_w_0.d/evtGen_1.root
    -s ../Stage1_EventGeneration_w_0.d/evtGen_2.root
    -s ../Stage1_EventGeneration_w_0.d/evtGen_3.root
    -s ../Stage1_EventGeneration_w_0.d/evtGen_4.root
  DATAFILES fcl/particleSimulation_w.fcl
)

set_tests_properties(Stage2_ParticleSimulation_w_0 PROPERTIES DEPENDS Stage1_EventGeneration_w_0)
set_tests_properties(Stage2_ParticleSimulation_w_1 PROPERTIES DEPENDS Stage1_EventGeneration_w_0)
set_tests_properties(Stage2_ParticleSimulation_w_2 PROPERTIES DEPENDS Stage1_EventGeneration_w_0)
set_tests_properties(Stage2_ParticleSimulation_w_3 PROPERTIES DEPENDS Stage1_EventGeneration_w_0)

cet_test(Stage3_Reconstruction_w HANDBUILT
  TEST_EXEC art
  PARG_J -j ${NUM_THREADS}
  TEST_ARGS --rethrow-all -c reconstruction_w.fcl
    -s "../Stage2_ParticleSimulation_w_0.d/sim_1.root"
    -s "../Stage2_ParticleSimulation_w_0.d/sim_2.root"
    -s "../Stage2_ParticleSimulation_w_0.d/sim_3.root"
    -s "../Stage2_ParticleSimulation_w_0.d/sim_4.root"
  DATAFILES fcl/reconstruction_w.fcl
)

set_tests_properties(Stage3_Reconstruction_w_0 PROPERTIES DEPENDS Stage2_ParticleSimulation_w_0)
set_tests_properties(Stage3_Reconstruction_w_1 PROPERTIES DEPENDS Stage2_ParticleSimulation_w_0)
set_tests_properties(Stage3_Reconstruction_w_2 PROPERTIES DEPENDS Stage2_ParticleSimulation_w_0)
set_tests_properties(Stage3_Reconstruction_w_3 PROPERTIES DEPENDS Stage2_ParticleSimulation_w_0)

# TEST PATH 1

cet_test(Stage4_Concatenation_w HANDBUILT
  TEST_EXEC art
  PARG_J -j ${NUM_THREADS}
  TEST_ARGS --rethrow-all -c concatenate_w.fcl
    -s "../Stage3_Reconstruction_w_0.d/reco_1.root"
    -s "../Stage3_Reconstruction_w_0.d/reco_2.root"
    -s "../Stage3_Reconstruction_w_0.d/reco_3.root"
    -s "../Stage3_Reconstruction_w_0.d/reco_4.root"
    -s "../Stage3_Reconstruction_w_0.d/reco_5.root"
    -s "../Stage3_Reconstruction_w_0.d/reco_6.root"
  DATAFILES fcl/concatenate_w.fcl
)

set_tests_properties(Stage4_Concatenation_w_0 PROPERTIES DEPENDS Stage3_Reconstruction_w_0)
set_tests_properties(Stage4_Concatenation_w_1 PROPERTIES DEPENDS Stage3_Reconstruction_w_0)
set_tests_properties(Stage4_Concatenation_w_2 PROPERTIES DEPENDS Stage3_Reconstruction_w_0)
set_tests_properties(Stage4_Concatenation_w_3 PROPERTIES DEPENDS Stage3_Reconstruction_w_0)

# A test using art_ut cannot use PARG_J -j ${NUM_THREADS}
# because those arguments get added before the TEST_ARGS,
# so they come before the "--" argument to art_ut which
# does not work.
cet_test(Stage5_Analysis_r HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS -- -c checkProducts_r.fcl
    -s "../Stage4_Concatenation_w_0.d/concatenation.root"
  DATAFILES fcl/checkProducts_r.fcl
  TEST_PROPERTIES
  DEPENDS Stage4_Concatenation_w_0
)

# TEST PATH 2

# A test using art_ut cannot use PARG_J -j ${NUM_THREADS}
# because those arguments get added before the TEST_ARGS,
# so they come before the "--" argument to art_ut which
# does not work.
cet_test(Stage4_Assemble_w HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS -- -c assemble_w.fcl
    -s "../Stage3_Reconstruction_w_0.d/reco_1.root"
    -s "../Stage3_Reconstruction_w_0.d/reco_2.root"
    -s "../Stage3_Reconstruction_w_0.d/reco_3.root"
    -s "../Stage3_Reconstruction_w_0.d/reco_4.root"
    -s "../Stage3_Reconstruction_w_0.d/reco_5.root"
    -s "../Stage3_Reconstruction_w_0.d/reco_6.root"
  DATAFILES fcl/assemble_w.fcl
  TEST_PROPERTIES
  DEPENDS Stage3_Reconstruction_w_0
)

# A test using art_ut cannot use PARG_J -j ${NUM_THREADS}
# because those arguments get added before the TEST_ARGS,
# so they come before the "--" argument to art_ut which
# does not work.
cet_test(Stage5_MoreAnalysis_r HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS -- -c checkMoreProducts_r.fcl
    -s "../Stage4_Assemble_w.d/userAssembled.root"
  DATAFILES fcl/checkMoreProducts_r.fcl
  TEST_PROPERTIES
  DEPENDS Stage4_Assemble_w
)

# A test using art_ut cannot use PARG_J -j ${NUM_THREADS}
# because those arguments get added before the TEST_ARGS,
# so they come before the "--" argument to art_ut which
# does not work.
cet_test(Stage4_Assemble_reordered_w HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS -- -c assemble_reordered_files_w.fcl
    -s "../Stage3_Reconstruction_w_0.d/reco_5.root"
    -s "../Stage3_Reconstruction_w_0.d/reco_6.root"
    -s "../Stage3_Reconstruction_w_0.d/reco_1.root"
    -s "../Stage3_Reconstruction_w_0.d/reco_2.root"
    -s "../Stage3_Reconstruction_w_0.d/reco_3.root"
    -s "../Stage3_Reconstruction_w_0.d/reco_4.root"
  DATAFILES fcl/assemble_reordered_files_w.fcl fcl/assemble_w.fcl
  TEST_PROPERTIES
  DEPENDS Stage3_Reconstruction_w_0
)

# A test using art_ut cannot use PARG_J -j ${NUM_THREADS}
# because those arguments get added before the TEST_ARGS,
# so they come before the "--" argument to art_ut which
# does not work.
cet_test(Stage5_MoreAnalysis_reordered_r HANDBUILT
  TEST_EXEC art_ut
  TEST_ARGS -- -c checkMoreProducts_r.fcl
    -s "../Stage4_Assemble_reordered_w.d/userAssembled.root"
    --process-name checkMoreProductsReordered
  DATAFILES fcl/checkMoreProducts_reordered_files_r.fcl fcl/checkMoreProducts_r.fcl
  TEST_PROPERTIES
  DEPENDS Stage4_Assemble_reordered_w
)

