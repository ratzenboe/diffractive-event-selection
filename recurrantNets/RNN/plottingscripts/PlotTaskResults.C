#include "TString.h"

#include "PlotTask.h"

void PlotBgTask(TString fname)
{
    PlotTask pt(fname, "BG");
    
    // create a combined LS plot
    pt.AddHists("Like sign (+-)", "fInvMass_LS_plus", "fInvMass_LS_minus");

    // ratio plots
    pt.PlotRatio("fInvMass_FD", "fInvMass_3plusTrks", "fInvMass_GammaDet_bg", "Like sign (+-)");
    pt.PlotRatio("fInvMass_FD", "fInvMass_3plusTrks", "fInvMass_GammaDet_bg");
    pt.PlotRatio("fInvMass_FD", "fInvMass_LS_plus", "fInvMass_LS_minus");
    pt.PlotRatio("fInvMass_FD", "fInvMass_LS_plus", "fInvMass_LS_minus", "Like sign (+-)");
    pt.PlotRatio("fInvMass_FD", "fInvMass_3plusTrks", "fInvMass_4trks", 
                 "fInvMass_5trks", "fInvMass_6trks");
    pt.PlotRatio("fInvMass_FD_hasGammas", "fInvMass_GammaDet_bg");
    // addative plots
    pt.PlotAdd("fInvMass_FD", "fInvMass_FD_emcal", "fInvMass_FD_3plus", "fInvMass_FD_other");
    pt.PlotAdd("fInvMass_3plusTrks", "fInvMass_3trks", "fInvMass_4trks", 
               "fInvMass_5trks", "fInvMass_6trks");

    // plot the same plots logarithmically
    pt.SetLog(kTRUE);
    // ratio plots
    pt.PlotRatio("fInvMass_FD", "fInvMass_3plusTrks", "fInvMass_GammaDet_bg", "Like sign (+-)");
    pt.PlotRatio("fInvMass_FD", "fInvMass_3plusTrks", "fInvMass_GammaDet_bg");
    pt.PlotRatio("fInvMass_FD", "fInvMass_LS_plus", "fInvMass_LS_minus");
    pt.PlotRatio("fInvMass_FD", "fInvMass_LS_plus", "fInvMass_LS_minus", "Like sign (+-)");
    pt.PlotRatio("fInvMass_FD", "fInvMass_3plusTrks", "fInvMass_4trks", 
                 "fInvMass_5trks", "fInvMass_6trks");
    pt.PlotRatio("fInvMass_FD_hasGammas", "fInvMass_GammaDet_bg");
    // addative plots
    pt.PlotAdd("fInvMass_FD", "fInvMass_FD_emcal", "fInvMass_FD_3plus", "fInvMass_FD_other");
    pt.PlotAdd("fInvMass_3plusTrks", "fInvMass_3trks", "fInvMass_4trks", 
               "fInvMass_5trks", "fInvMass_6trks");
}

void PlotEMCalTask(TString fname_hitfiles, TString fname_rest)
{
    PlotTask pt_hit(fname_hitfiles, "EMCAL");
    // we only make one plot with the hit files, for the other ones we use the full statistic
    pt_hit.SetAxisRange(0.,3.);
    pt_hit.PlotRatio("fGammaE", "fSecondaryE_BG");

    // log
    pt_hit.SetLog(kTRUE);
    pt_hit.PlotRatio("fGammaE", "fSecondaryE_BG");

    //////////////////////////////////////////////////////////////
    // now the plots with the full statistic
    PlotTask pt_full(fname_rest, "EMCAL");

    pt_full.SetAxisRange(-1.e-7, 1.e-7);
    pt_full.PlotSigBg("fClusterTime_SIG", "fClusterTime_BG");
    pt_full.SetAxisRange(0.,3.);
    pt_full.PlotSigBg("fEnergy_SIG", "fEnergy_BG");
    pt_full.SetAxisRange(0.,5.);
    pt_full.PlotSigBg("fdPhiEta_pion", "fdPhiEta_gamma");

    pt_full.SetLog(kTRUE);
    pt_full.SetAxisRange(-1.e-7, 1.e-7);
    pt_full.PlotSigBg("fClusterTime_SIG", "fClusterTime_BG");
    pt_full.SetAxisRange(0.,3.);
    pt_full.PlotSigBg("fEnergy_SIG", "fEnergy_BG");
    pt_full.SetAxisRange(0.,5.);
    pt_full.PlotSigBg("fdPhiEta_pion", "fdPhiEta_gamma");
}

