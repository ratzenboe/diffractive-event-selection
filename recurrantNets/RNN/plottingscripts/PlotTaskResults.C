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
    pt_hit.SetXaxisText("E (GeV)");
    pt_hit.SetYaxisText("Counts / (0.04 GeV)");
    pt_hit.PlotSigBg("fGammaE", "fSecondaryE_BG");

    // log
    pt_hit.SetLog(kTRUE);
    pt_hit.SetLegendPos(0.187414,0.188912,0.421853,0.397331);
    pt_hit.PlotSigBg("fGammaE", "fSecondaryE_BG");
    pt_hit.ResetLegendPos();

    //////////////////////////////////////////////////////////////
    // now the plots with the full statistic
    PlotTask pt_full(fname_rest, "EMCAL");

    pt_full.SetLog(kTRUE);
    pt_full.SetAxisRange(0.,1.3);
    pt_full.SetTextPos(0.711745,4137.76);
    pt_full.SetLegendPos(0.592669, 0.477413,0.827109,0.667351);
    pt_full.SetXaxisText("E (GeV)");
    pt_full.SetYaxisText("Counts / (0.03 GeV)");
    pt_full.PlotSigBg("fEnergy_SIG", "fEnergy_BG");
    pt_full.ResetTextPos();
    pt_full.SetAxisRange(0.,5.);
    pt_full.SetLegendPos(0.587137,0.552361,0.821577,0.709446);
    pt_full.SetXaxisText("Cluster distance to nearest track in #phi-#eta space");
    pt_full.SetYaxisText("Counts");
    pt_full.PlotSigBg("fdPhiEta_pion", "fdPhiEta_gamma");
}

