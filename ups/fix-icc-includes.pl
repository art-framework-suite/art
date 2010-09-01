use strict;
use vars qw(%translations);
BEGIN { %translations = (
                         "FWCore/Framework/interface/eventSetupGetImplementation.icc" => "art/Framework/Core/eventSetupGetImplementation.icc",
                         "FWCore/Framework/interface/HCMethods.icc" => "art/Framework/Core/HCMethods.icc",
                         "FWCore/Framework/interface/HCTypeTag.icc" => "art/Framework/Core/HCTypeTag.icc",
                         "FWCore/Framework/interface/HCTypeTagTemplate.icc" => "art/Framework/Core/HCTypeTagTemplate.icc",
                         "FWCore/Framework/interface/recordGetImplementation.icc" => "art/Framework/Core/recordGetImplementation.icc",
                         "FWCore/MessageService/interface/ELtsErrorLog.icc" => "art/Framework/Services/Message/ELtsErrorLog.icc",
                         "FWCore/MessageService/interface/ErrorLog.icc" => "art/Framework/Services/Message/ErrorLog.icc",
                         "FWCore/MessageLogger/interface/ELseverityLevel.icc" => "art/MessageLogger/ELseverityLevel.icc",
                         "FWCore/MessageLogger/interface/ErrorObj.icc" => "art/MessageLogger/ErrorObj.icc",
                         "FWCore/MessageLogger/interface/ThreadSafeErrorLog.icc" => "art/MessageLogger/ThreadSafeErrorLog.icc"
                        );
      }
foreach my $inc (sort keys %translations) {
  s&^(\s*#include\s+["<])\Q$inc\E([">].*)$&${1}$translations{$inc}${2}&;
}

### Local Variables:
### mode: cperl
### End:
