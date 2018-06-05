/* Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. */
/* See cxx source for full Copyright notice */
/* $Id$ */

#ifndef AliAnalysisTaskMCInfo_H
#define AliAnalysisTaskMCInfo_H

#include "TBits.h"
#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TList.h"
#include "TBranch.h"
#include "TDirectory.h"
#include "TClonesArray.h"
#include "TString.h"
#include "TList.h"
#include "TArrayI.h"
#include "TObjArray.h"
#include "AliESDEvent.h"
#include "AliMCEvent.h"
#include "AliESDCaloCluster.h"
#include "AliTriggerAnalysis.h"
#include "AliCEPUtils.h"
#include "AliPIDResponse.h"
#include "AliPIDCombined.h"

#include "EventStorage.h"
#include "EventDef.h"

#include "AliAnalysisTaskSE.h"
#include "CEPBGBase.h"

class AliAnalysisTaskBG3plus : public AliAnalysisTaskSE, public CEPBGBase   
{
    public:
                                AliAnalysisTaskBG3plus();
                                AliAnalysisTaskBG3plus(const char *name,
                                                       Long_t state,
                                                       UInt_t TTmask,
                                                       UInt_t TTpattern,
                                                       TString hitFileName);
        virtual                 ~AliAnalysisTaskBG3plus();

        // these functions also exist in AliAnalysisTaskSE (SE=single event)
        // as we want to change the functions a litte, but dont want to change the 
        // original ones, we make them 'virtual' functions
        virtual void            UserCreateOutputObjects();
        virtual void            UserExec(Option_t* option);
        virtual void            Terminate(Option_t* option);

    private:
        /* // Output objects */ 
        TList*                  fOutList;           //! output list
        TH1F*                   fInvMass_FD;        //! invariant mass of feed down evts
        TH1F*                   fInvMass_3trks;     //! invariant mass of 3 tracks bg
        TH1F*                   fInvMass_3plusTrks; //! invariant mass of 3+ tracks (here: 3-10)
        
        // not implemented but neccessary
        AliAnalysisTaskBG3plus(const AliAnalysisTaskBG3plus&); 
        AliAnalysisTaskBG3plus& operator=(const AliAnalysisTaskBG3plus&); 


        ClassDef(AliAnalysisTaskBG3plus, 1);
};

#endif
