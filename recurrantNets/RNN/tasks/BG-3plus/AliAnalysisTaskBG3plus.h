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
#include "AliEMCALGeometry.h"

#include "EventStorage.h"
#include "EventDef.h"

#include "AliAnalysisTaskSE.h"

class AliAnalysisTaskBG3plus : public AliAnalysisTaskSE  
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
        AliESDEvent*            fESD;               //! input event
        AliTriggerAnalysis*     fTrigger;           //! trigger object
        TArrayI*                fTrackStatus;       //! array of track-status
        TObjArray*              fTracks;            //! array of AliESDtracks
        AliCEPUtils*            fCEPUtil;           //! AliCEPUtil object

        Long_t                  fAnalysisStatus;    //  stores the analysis-status 
        UInt_t                  fTTmask;            //  track conditions
        UInt_t                  fTTpattern;         //  track conditions
        // Output objects 
        TList*                  fOutList;           //! output list
        TH1F*                   fInvMass_FD;        //! invariant mass of feed down evts
        TH1F*                   fInvMass_3trks;     //! invariant mass of 3 tracks bg
        
        // not implemented but neccessary
        AliAnalysisTaskBG3plus(const AliAnalysisTaskBG3plus&); 
        AliAnalysisTaskBG3plus& operator=(const AliAnalysisTaskBG3plus&); 

        // prefiltering 
        Bool_t                  lhc16filter(AliESDEvent* esd_evt, Int_t nTracksAccept, 
                                            Int_t& nTracksTT, TArrayI*& TTindices);
        Bool_t                  lhc16filter(AliESDEvent* esd_evt, std::vector<Int_t> nTrksAcc_vec, 
                                            Int_t& nTracksTT, TArrayI*& TTindices);
         // heart of the lhc16-filter
        Bool_t                  EventFilter(AliESDEvent* esd_evt, Int_t nTracksAccept);
        // part of the lhc16filter
        Bool_t                  IsSTGFired(TBits* fFOmap,Int_t dphiMin=0,Int_t dphiMax=10);

        // check if event is fully reconstructed
        Bool_t                  EvtFullRecon(TObjArray* tracks, Int_t nTracksTT, 
                                             TArrayI* TTindices, AliMCEvent* MCevt) const;
        TLorentzVector          GetXLorentzVector(AliMCEvent* MCevent) const;

        // get lorentz vector of particle index
        TParticle*              GetPartByLabel(Int_t MCind, AliMCEvent* MCevt) const;
        // get mass of event
        Double_t                GetMass(TObjArray* tracks, Int_t nTracksTT, 
                                        TArrayI* TTindices, AliMCEvent* MCevt) const;
        std::vector<Double_t>   GetMassPermute(TObjArray* tracks, Int_t nTracksTT, 
                                               TArrayI* TTindices, AliMCEvent* MCevt) const;
        //////////////////////////////////////////////////////////////////////////////////
        // ------------------------------ Print functions --------------------------------
        // print particle stack
        void                    PrintStack(AliMCEvent* MCevent, Bool_t prim=kTRUE) const;
        // print particle stack
        void                    PrintTracks(AliESDEvent* esd_evt) const;

        ClassDef(AliAnalysisTaskBG3plus, 1);
};

#endif
