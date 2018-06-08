/* Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. */
/* See cxx source for full Copyright notice */
/* $Id$ */

#ifndef AliAnalysisTaskBG_H
#define AliAnalysisTaskBG_H

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
#include "AliEMCALGeometry.h"

#include "EventStorage.h"
#include "EventDef.h"
#include "CEPBGBase.h"

#include "AliAnalysisTaskSE.h"

class AliAnalysisTaskBG : public AliAnalysisTaskSE, public CEPBGBase
{
    public:
                                AliAnalysisTaskBG();
                                AliAnalysisTaskBG(const char *name,
                                                      Long_t state,
                                                      UInt_t TTmask,
                                                      UInt_t TTpattern,
                                                      TString hitFileName);
        virtual                 ~AliAnalysisTaskBG();

        // these functions also exist in AliAnalysisTaskSE (SE=single event)
        // as we want to change the functions a litte, but dont want to change the 
        // original ones, we make them 'virtual' functions
        virtual void            UserCreateOutputObjects();
        virtual void            UserExec(Option_t* option);
        virtual void            Terminate(Option_t* option);

    private:
        // Output objects 
        TList*                  fOutList;               //! output list
        TH1F*                   fInvMass_FD;        //! invariant mass of feed down evts
        TH1F*                   fInvMass_3trks;     //! invariant mass of 3 tracks bg
        TH1F*                   fInvMass_3plusTrks; //! invariant mass of 3+ tracks (here: 3-10)
        TH1F*                   fInvMass_GammaDet;  //! invariant mass of gamma in emcal

       
        // not implemented but neccessary
        AliAnalysisTaskBG(const AliAnalysisTaskBG&); 
        AliAnalysisTaskBG& operator=(const AliAnalysisTaskBG&); 

        ClassDef(AliAnalysisTaskBG, 1);
};

#endif
