#!/usr/bin/perl -w

use strict;

my @signals =
  (
   "PostBeginJob",
   "PostEndJob",
   "JobFailure",
   "PreSource",
   "PostSource",
   "PreSourceSubRun",
   "PostSourceSubRun",
   "PreSourceRun",
   "PostSourceRun",
   "PreOpenFile",
   "PostOpenFile",
   "PreCloseFile",
   "PostCloseFile",
   "PreProcessEvent",
   "PostProcessEvent",
   "PreBeginRun",
   "PostBeginRun",
   "PreEndRun",
   "PostEndRun",
   "PreBeginSubRun",
   "PostBeginSubRun",
   "PreEndSubRun",
   "PostEndSubRun",
   "PreProcessPath",
   "PostProcessPath",
   "PrePathBeginRun",
   "PostPathBeginRun",
   "PrePathEndRun",
   "PostPathEndRun",
   "PrePathBeginSubRun",
   "PostPathBeginSubRun",
   "PrePathEndSubRun",
   "PostPathEndSubRun",
   "PreModuleConstruction",
   "PostModuleConstruction",
   "PostBeginJobWorkers",
   "PreModuleBeginJob",
   "PostModuleBeginJob",
   "PreModuleEndJob",
   "PostModuleEndJob",
   "PreModule",
   "PostModule",
   "PreModuleBeginRun",
   "PostModuleBeginRun",
   "PreModuleEndRun",
   "PostModuleEndRun",
   "PreModuleBeginSubRun",
   "PostModuleBeginSubRun",
   "PreModuleEndSubRun",
   "PostModuleEndSubRun",
   "PostServiceReconfigure"
  );

foreach my $signal (@signals) {
  s&(->|\.)watch\Q$signal\E\b&$1s$signal\.watch&g;
  s&s\Q$signal\E_&s$signal\.invoke&g;
}
