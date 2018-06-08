// runCEP.C
//
// run macro for CEP data analysis
//
// Author: Paul Buehler <paul.buehler@oeaw.ac.at>
//
// LHC10b  LHC14j4b
// LHC10c  LHC14j4c
// LHC10d  LHC14j4d
// LHC10e  LHC14j4e
// LHC10f  LHC14j4f
//
// LHC16d  LHC17f6
// LHC16e  LHC17f9
// LHC16h  LHC17f5
// LHC16i  LHC17d3
// LHC16j  LHC17e5
// LHC16k  LHC16j2a1, LHC17d20a1, LHC17d20a1_extra
// LHC16l  LHC16j2a2, LHC17d20a2, LHC17d20a2_extra
// LHC16o  LHC17d16
// LHC16p  LHC17d18

class AliAnalysisGrid;

//______________________________________________________________________________
void runEMCALAna (
  const char* runtype           = "local",     // local, proof or grid
  const char *gridmode          = "full",     // Set the run mode (can be "full", "test", "offline", "submit" or "terminate"). Full & Test work for proof
  const bool isMC               = kTRUE,      // kTRUE = looking at MC truth or reconstructed, 0 = looking at real data
  const bool enableBGrejection  = kTRUE,      // apply BG rejection in physics selection
  const Long64_t nentries       = 1e7,          // for local and proof mode, ignored in grid mode. Set to 1234567890 for all events.
  const Long64_t firstentry     = 0,          // for local and proof mode, ignored in grid mode
  const char *proofdataset      = "/alice/sim/LHC10c_000120821_p1", // path to dataset on proof cluster, for proof analysis
  const char *proofcluster      = "alice-caf.cern.ch",              // which proof cluster to use in proof mode
  const char *griddatapattern   = "*/AliESDs.root",                // Data Pattern
  const char *option            = "" )
{
	// set taskname
  TString tn2u = TString("LHC");
  tn2u.Append(option);
  if (TString(option).EqualTo("")) tn2u = TString("Pythia8CEP");
  printf("task name: %s\n",tn2u.Data());

  // set data path on grid
  Bool_t isRun1 = kFALSE;
  Bool_t isLHC16 = kFALSE;
  TString gp2u = TString("/alice/sim/");
  if (TString(option).BeginsWith("10")) {
    isRun1 = kTRUE;
    griddatapattern = "*/AliESDs.root";
    gp2u.Append("2010/");
  } else if (TString(option).BeginsWith("14")) {
    isRun1 = kTRUE;
    griddatapattern = "*/AliESDs.root";
    gp2u.Append("2014/");
  } else if (TString(option).BeginsWith("16")) {
    isLHC16 = kTRUE;
    griddatapattern = "*/AliESDs.root";
    gp2u.Append("2016/");
  } else if (TString(option).BeginsWith("17")) {
    isLHC16 = kTRUE;
    griddatapattern = "*/AliESDs.root";
    gp2u.Append("2017/");
  }
  gp2u.Append("LHC");
  gp2u.Append(option);

  if (TString(option).EqualTo("")) {
    // gp2u = TString("/alice/cern.ch/user/e/ekryshen/pp/pythia8");
    gp2u = TString("/alice/cern.ch/user/p/pbuhler/CEP/Pythia8CEP");
    griddatapattern = "*/AliESDs.root";
  }
  printf("gridpath: %s\n",gp2u.Data());
  
  // --------------------------------------------------------------------------
  // Check run type
	if(runtype != "local" && runtype != "proof" && runtype != "grid"){
		Printf("\n\tIncorrect run option, check first argument of run macro");
		Printf("\tint runtype = local, proof or grid\n");
		return;
	}
	Printf("%s analysis chosen",runtype);

  // --------------------------------------------------------------------------
	gROOT->ProcessLine(Form(".include %s/include",gSystem->ExpandPathName("$ROOTSYS")));
	gROOT->ProcessLine(Form(".include %s/include",gSystem->ExpandPathName("$ALICE_ROOT")));
	gROOT->ProcessLine(Form(".include %s/include",gSystem->ExpandPathName("$ALICE_PHYSICS")));
	gROOT->SetStyle("Plain");

	Printf("========== Successing Loading include path ==========");

  // --------------------------------------------------------------------------
	// Create the alien handler and attach it to the manager
  printf("\nDEBUG: before CreateAlienHandler_devel()\n");
	/* gROOT->LoadMacro("CreateAlienHandler_devel.C"); */
	gROOT->LoadMacro("CreateAlienHandler_MC.C");
	AliAnalysisGrid *plugin = CreateAlienHandler_MC
  (
    tn2u.Data(), gridmode, proofcluster, proofdataset,
    gp2u.Data(), griddatapattern, option, isMC
  );

	Printf("========== Succesing Loading Plugin ==========");

  // --------------------------------------------------------------------------
	// Analysis manager
	AliAnalysisManager* mgr = new AliAnalysisManager("EMCAL-Manager");
	mgr->SetGridHandler(plugin);

	AliESDInputHandler* esdH = new AliESDInputHandler();
  esdH->SetNeedField(kTRUE);
	mgr->SetInputEventHandler(esdH);

  printf("\nDEBUG: after inputhandler creation\n");
  // --------------------------------------------------------------------------
	// Physics selection task
	// in case of PYTHIA disable PhysicsSelection
  // this is important to get MC truth of all simulated CD events
  // instead set isSaveAll=kTRUE

  // run without physics selection
  Bool_t withPhysSel      = kFALSE;
  Bool_t withDGTriggerSel = kFALSE;
  Bool_t withEMCalCorrection = kTRUE;
  
	gROOT->LoadMacro("$ALICE_PHYSICS/OADB/macros/AddTaskPhysicsSelection.C");
	AliPhysicsSelectionTask *physicsSelectionTask = AddTaskPhysicsSelection(isMC,enableBGrejection);
	//AliPhysicsSelectionTask *physicsSelectionTask = AddTaskPhysicsSelection();
	if(!physicsSelectionTask) { Printf("no physSelTask"); return; }
  printf("\nDEBUG: after AddTaskPhysicsSelection\n");

  // create a custom trigger mask for the physics selection
  if (isLHC16 && withDGTriggerSel) {
    AliOADBPhysicsSelection * oadbLHC16CEP = new AliOADBPhysicsSelection("oadbLHC16CEP");
    oadbLHC16CEP->AddCollisionTriggerClass( AliVEvent::kDG,"+CCUP13-B-SPD1-CENTNOTRD","B",0 );
    oadbLHC16CEP->SetHardwareTrigger(0,"SPDGFO>=1 && !V0A && !V0C");
    oadbLHC16CEP->SetOfflineTrigger(0,"(SPDGFO>=1 && !V0A && !V0C) &&!V0ABG&&!V0CBG&&!TPCLaserWarmUp");     
    physicsSelectionTask->GetPhysicsSelection()->SetCustomOADBObjects(oadbLHC16CEP,0);
  }
  
  // --------------------------------------------------------------------------
    // Add PID task manager if we use PID
    gROOT->LoadMacro("$ALICE_ROOT/ANALYSIS/macros/AddTaskPIDResponse.C");
    AliAnalysisTaskPIDResponse *pidResponseTask = AddTaskPIDResponse(isMC);
    if(!pidResponseTask) { Printf("no pidResponseTask"); return; }

  printf("\nDEBUG: after AddTaskPIDResponse\n");
    // --------------------------------------------------------------------------
    // EMCal correction task
    // The default argument is an empty string, so we don't have to set it here.
    if (withEMCalCorrection) {
        gROOT->LoadMacro("$ALICE_PHYSICS/PWG/EMCAL/macros/AddTaskEmcalCorrectionTask.C");
        AliEmcalCorrectionTask * correctionTask = AddTaskEmcalCorrectionTask();
        if(!correctionTask) { Printf("no EmcalCorrectionTask"); return; }
        correctionTask->SetUserConfigurationFilename("AliEmcalCorrectionConfiguration_CEP.yaml");
        correctionTask->Initialize();
    }
 
  // --------------------------------------------------------------------------
  // add the analysis task
  // taskconfig: task configuration
  // numTracksMax: maximum number of selected tracks an event can have to be
  //               saved
  // ETmaskDG, ETpatternDG: mask and pattern to test fCurrentGapCondition
  // TTmask, TTpattern: mask and pattern to test fTrackStatus
  //                    isok = (stat & mask) == pattern

  // with taskconfig the functionality of AliAnalysisTaskCEP can be set
  UInt_t taskconfig = AliCEPBase::kBitConfigurationSet;
  if (isRun1) taskconfig |= AliCEPBase::kBitisRun1;
  if (isMC)   taskconfig |= AliCEPBase::kBitisMC;
  taskconfig |= AliCEPBase::kBitSaveAllEvents;
  taskconfig |= AliCEPBase::kBitQArnumStudy;
  //taskconfig |= AliCEPBase::kBitBBFlagStudy;
  //taskconfig |= AliCEPBase::kBitSPDPileupStudy;
  //taskconfig |= AliCEPBase::kBitnClunTraStudy;
  // taskconfig |= AliCEPBase::kBitVtxStudy;
  // taskconfig |= AliCEPBase::kBitV0Study;
  //taskconfig |= AliCEPBase::kBitFMDStudy;
  //taskconfig |= AliCEPBase::kBitTrackCutStudy;
  // (un)comment next line to toggle raw-buffer saving
  taskconfig |= AliCEPBase::kBitRawBuffer;
  printf("TaskConfiguration: %i\n",taskconfig);

  Int_t rnummin, rnummax;
  Int_t numTracksMax;
  Double_t fracDG, fracNDG;
  UInt_t ETmaskDG,ETpatternDG,ETmaskNDG,ETpatternNDG,TTmask,TTpattern;
  getTaskOptions( option,
    rnummin,rnummax,
    numTracksMax,
    fracDG,fracNDG,
    ETmaskDG,ETpatternDG,ETmaskNDG,ETpatternNDG,
    TTmask,TTpattern
  );

  printf("\nDEBUG: before AddEMCALTask()\n");
  // hit file is EMCAL.Hits.root  
  TString hitfile("EMCAL.Hits.root");
  gROOT->LoadMacro("AliAnalysisTaskEMCAL.cxx++g");
  gROOT->LoadMacro("AddEMCALTask.C");
  AliAnalysisTaskSE* task = AddEMCALTask (tn2u,taskconfig, TTmask,TTpattern,hitfile);
  
  // activate physics selection
  if (withPhysSel) {
    if (isLHC16 && withDGTriggerSel) {
      task->SelectCollisionCandidates(AliVEvent::kDG);
    } else {
      task->SelectCollisionCandidates();
    }
  }
	  
  // --------------------------------------------------------------------------
  // Start analysis
	mgr->SetDebugLevel(0);
	//mgr->SetNSysInfo(10);
	if (!mgr->InitAnalysis()) return;
	mgr->PrintStatus();
	Printf("Starting Analysis....");
	
  mgr->StartAnalysis(runtype,nentries,firstentry);
	// mgr->StartAnalysis(runtype);
  
}

// -----------------------------------------------------------------------------
void getTaskOptions (char *option,
  Int_t& rnummin,
  Int_t& rnummax,
  Int_t& numTracksMax,
  Double_t& fracDG,
  Double_t& fracNDG,
  UInt_t& ETmaskDG,
  UInt_t& ETpatternDG,
  UInt_t& ETmaskNDG,
  UInt_t& ETpatternNDG,
  UInt_t& TTmask,
  UInt_t& TTpattern )
{
  
  if (TString(option).BeginsWith("14")) {
    rnummin      = 114000;
    rnummax      = 135000;
    numTracksMax = 10;
    fracDG       = 1.0;
    fracNDG      = 0.05;
    ETmaskDG     = AliCEPBase::kETPileup |
                   AliCEPBase::kETVtx    |
                   AliCEPBase::kETV0A    |
                   AliCEPBase::kETV0C;
    ETpatternDG  = AliCEPBase::kETVtx;
    ETmaskNDG    = AliCEPBase::kETVtx;
    ETpatternNDG = AliCEPBase::kETVtx;
    TTmask       = AliCEPBase::kTTAccITSTPC;
    TTpattern    = AliCEPBase::kTTAccITSTPC;

  } else if (TString(option).BeginsWith("15")) {
    rnummin      = 224000;
    rnummax      = 232000;
    numTracksMax = 10;
    fracDG       = 1.0;
    fracNDG      = 0.05;
    ETmaskDG     = AliCEPBase::kETPileup |
                   AliCEPBase::kETVtx    |
                   AliCEPBase::kETV0A    |
                   AliCEPBase::kETV0C;
    ETpatternDG  = AliCEPBase::kETVtx;
    ETmaskNDG    = AliCEPBase::kETVtx;
    ETpatternNDG = AliCEPBase::kETVtx;
    TTmask       = AliCEPBase::kTTAccITSTPC;
    TTpattern    = AliCEPBase::kTTAccITSTPC;

  } else if (TString(option).BeginsWith("16")) {
    rnummin      = 252000;
    rnummax      = 265000;
    numTracksMax = 10;
    fracDG       = 1.0;
    fracNDG      = 0.05;
    ETmaskDG     = AliCEPBase::kETDGTrigger |
                   AliCEPBase::kETV0A       |
                   AliCEPBase::kETV0C;
    ETpatternDG  = AliCEPBase::kETDGTrigger;
    ETmaskNDG    = 0;
    ETpatternNDG = 0;
    TTmask       = AliCEPBase::kTTAccTPCOnly |
                   AliCEPBase::kTTSPDHit;
    TTpattern    = AliCEPBase::kTTAccTPCOnly |
                   AliCEPBase::kTTSPDHit;

  } else if (TString(option).BeginsWith("17")) {
    rnummin      = 252000;
    rnummax      = 265000;
    numTracksMax = 10;
    fracDG       = 1.0;
    fracNDG      = 0.05;
    ETmaskDG     = AliCEPBase::kETDGTrigger |
                   AliCEPBase::kETV0A       |
                   AliCEPBase::kETV0C;
    ETpatternDG  = AliCEPBase::kETDGTrigger;
    ETmaskNDG    = 0;
    ETpatternNDG = 0;
    TTmask       = AliCEPBase::kTTAccTPCOnly |
                   AliCEPBase::kTTSPDHit;
    TTpattern    = AliCEPBase::kTTAccTPCOnly |
                   AliCEPBase::kTTSPDHit;

  } else if (TString(option).EqualTo("")) {
    rnummin      = 252000;
    rnummax      = 265000;
    numTracksMax = 10;
    fracDG       = 1.0;
    fracNDG      = 0.05;
    ETmaskDG     = AliCEPBase::kETDGTrigger |
                   AliCEPBase::kETV0A       |
                   AliCEPBase::kETV0C;
    ETpatternDG  = AliCEPBase::kETDGTrigger;
    ETmaskNDG    = 0;
    ETpatternNDG = 0;
    TTmask       = AliCEPBase::kTTAccTPCOnly |
                   AliCEPBase::kTTSPDHit;
    TTpattern    = AliCEPBase::kTTAccTPCOnly |
                   AliCEPBase::kTTSPDHit;

  }
  
  printf("TaskOptions\n");
  printf("rnum min:     %i\n",rnummin);
  printf("rnum max:     %i\n",rnummax);
  printf("numTracksMax: %i\n",numTracksMax);
  printf("fracDG:       %f\n",fracDG);
  printf("fracNDG:      %f\n",fracNDG);
  printf("ETmaskDG:     %i\n",ETmaskDG);
  printf("ETpatternDG:  %i\n",ETpatternDG);
  printf("ETmaskNDG:    %i\n",ETmaskNDG);
  printf("ETpatternNDG: %i\n",ETpatternNDG);
  printf("TTmask:       %i\n",TTmask);
  printf("TTpattern:    %i\n\n",TTpattern);
  
  return;

}

// -----------------------------------------------------------------------------
