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
        // number of evts infos
        Int_t                   fAllEvts;           // all evts
        Int_t                   fnEvtsPassedFilter; // events that passed the filter
        Int_t                   fnEvtsTotal;        // events that have a mc event
        TString                 fFilesList;         // list of files
        // Output objects 
        TList*                  fOutList;           //! output list
        TH1F*                   fInvMass_FD;        //! invariant mass of feed down evts
        // feed down describing histograms
        TH1F*                   fInvMass_FD_emcal;  //! FD that only consists of extra gammas
        TH1F*                   fInvMass_FD_3plus;  //! FD that could be detectable by 3+ methods
        TH1F*                   fInvMass_FD_other;  //! FD not describable by any of the above 
        // special case is not summed up like the others but used only to show that this would
        // be a very precise bg approximation if the gammas could be detected more efficiently
        TH1F*                   fInvMass_FD_hasGammas;  //! FD that is accompanied by gammas

        // background approximations
        TH1F*                   fInvMass_3trks;     //! invariant mass of 3 tracks bg
        TH1F*                   fInvMass_4trks;     //! invariant mass of 4 tracks bg
        TH1F*                   fInvMass_5trks;     //! invariant mass of 5 tracks bg
        TH1F*                   fInvMass_6trks;     //! invariant mass of 6 tracks bg
        TH1F*                   fInvMass_3plusTrks; //! invariant mass of 3+ tracks (here: 3-10)
        TH1F*                   fInvMass_GammaDet_sig;  //! invariant mass of gamma in emcal sig
        TH1F*                   fInvMass_GammaDet_bg;   //! invariant mass of gamma in emcal sig
        // Likesign distribution
        TH1F*                   fInvMass_LS_plus;   //! invariant mass of LS+
        TH1F*                   fInvMass_LS_minus;  //! invariant mass of LS-
        // number of tracks passing through the lhc16filter
        TH1I*                   fNb_trks_passed;    //! nb tracks passing lhc16filter

       
        // not implemented but neccessary
        AliAnalysisTaskBG(const AliAnalysisTaskBG&); 
        AliAnalysisTaskBG& operator=(const AliAnalysisTaskBG&); 

        ClassDef(AliAnalysisTaskBG, 1);
};

#endif
