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

