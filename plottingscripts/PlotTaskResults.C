#include "TString.h"

#include "PlotTask.h"

void PlotBgTask(TString fname)
{
    PlotTask pt(fname, "BG");
    
    // create a combined LS plot
    pt.SetAxisRange(0.,2.7);
    pt.LikeSignHist("fInvMass_LS_plus", "fInvMass_LS_minus");

    // ratio plots
    /* pt.PlotRatio("fInvMass_FD", "fInvMass_3plusTrks", "fInvMass_GammaDet_bg", */ 
    /*              "Like sign (2 #sqrt{++ #times --})"); */
    /* pt.PlotRatio("fInvMass_FD", "fInvMass_3plusTrks", "fInvMass_GammaDet_bg"); */
    /* pt.PlotRatio("fInvMass_FD", "Like sign (2 #sqrt{++ #times --})"); */
    /* pt.PlotRatio("fInvMass_FD", "fInvMass_LS_plus", "fInvMass_LS_minus", */ 
    /*              "Like sign (2 #sqrt{++ #times --})"); */
    /* pt.PlotRatio("fInvMass_FD", "fInvMass_LS_plus", "fInvMass_LS_minus", */ 
    /*              "Like sign (2 #sqrt{++ #times --})"); */
    /* pt.PlotRatio("fInvMass_FD", "fInvMass_3plusTrks", "fInvMass_4trks", */ 
    /*              "fInvMass_5trks"); */
    /* pt.PlotRatio("fInvMass_FD_hasGammas", "fInvMass_GammaDet_bg"); */

    // addative plots
    /* pt.PlotAdd("fInvMass_FD", "fInvMass_FD_emcal", "fInvMass_FD_3plus", "fInvMass_FD_other"); */
    /* pt.PlotAdd("fInvMass_3plusTrks", "fInvMass_3trks", "fInvMass_4trks", "fInvMass_5trks"); */

    // like-sign >2 track FD comparison
    pt.PlotAdd("fInvMass_FD", "fInvMass_FD_hasGammas", "fInvMass_FD_emcal");


    // plot the same plots logarithmically
    pt.SetLog(kTRUE);
    // ratio plots
    pt.PlotRatio("fInvMass_FD", "fInvMass_3plusTrks", "fInvMass_GammaDet_bg", 
                 "LS (2 #sqrt{++ #times --} )");
    pt.PlotRatio("fInvMass_FD", "fInvMass_3plusTrks", "fInvMass_GammaDet_bg");
    pt.PlotRatio("fInvMass_FD", "fInvMass_LS_plus", "fInvMass_LS_minus");
    pt.PlotRatio("fInvMass_FD", "fInvMass_LS_plus", "fInvMass_LS_minus", 
                 "LS (2 #sqrt{++ #times --} )");
    pt.PlotRatio("fInvMass_FD", "fInvMass_3plusTrks", "fInvMass_4trks", 
                 "fInvMass_5trks");
    pt.PlotRatio("fInvMass_FD_hasGammas", "fInvMass_GammaDet_bg");
    // addative plots
    /* pt.PlotAdd("fInvMass_FD", "fInvMass_FD_emcal", "fInvMass_FD_3plus", "fInvMass_FD_other"); */
    /* pt.SetStdColors(kBlack, 6, 8, kBlue, kRed-4); */
    /* pt.PlotAdd("fInvMass_3plusTrks", "fInvMass_3trks", "fInvMass_4trks", */ 
    /*            "fInvMass_5trks", "fInvMass_6trks"); */
    /* pt.PlotRatio("fInvMass_3plusTrks", "fInvMass_3trks", "fInvMass_4trks", */ 
    /*              "fInvMass_5trks", "fInvMass_6trks"); */    
    /* pt.PlotAdd("fInvMass_FD", "fInvMass_FD_hasGammas", "fInvMass_FD_emcal"); */

    pt.ToggleDisplayCanvas();
    pt.SetStdColors(kBlack,6);
    pt.SetTextPos(1.48692, 66.7423);
    pt.SetLegendPos(0.224066, 0.0471266, 0.556708, 0.276112);
    pt.PlotRatio("fInvMass_FD", "fInvMass_3trks");
    // ADD
    /* pt.SetYaxisRange(0.3, 1.2e3); */
    pt.SetTextPos(1.48444, 77.0644);
    pt.SetLegendPos(0.569156,0.548087,0.751037,0.687694);
    pt.PlotAdd("fInvMass_FD", "fInvMass_3trks");


    // like-sign - >2 track FD comparison
    // RATIO
    /* pt.SetStdColors(kBlack,8); */
    /* pt.SetTextPos(1.48692, 66.7423); */
    /* pt.SetLegendPos(0.224066, 0.0471266, 0.556708, 0.276112); */
    /* pt.PlotRatio("fInvMass_FD", "fInvMass_GammaDet_bg"); */
    /* // ADD */
    /* pt.SetTextPos(1.48444, 77.0644); */
    /* pt.SetLegendPos(0.569156,0.548087,0.751037,0.687694); */
    /* pt.PlotAdd("fInvMass_FD", "fInvMass_GammaDet_bg"); */

    /* pt.ToggleDisplayCanvas(); */
    // like-sign - >2 track FD comparison
    // RATIO
    /* pt.ToggleDisplayCanvas(); */
    /* pt.SetStdColors(kBlack,9); */
    /* pt.SetTextPos(1.48692, 66.7423); */
    /* pt.SetLegendPos(0.224066, 0.0471266, 0.556708, 0.276112); */
    /* pt.PlotRatio("fInvMass_FD", "LS (2 #sqrt{++ #times --} )"); */
    /* // ADD */
    /* pt.SetTextPos(1.48444, 77.0644); */
    /* pt.SetLegendPos(0.569156,0.548087,0.751037,0.687694); */
    /* pt.PlotAdd("fInvMass_FD", "LS (2 #sqrt{++ #times --} )"); */

    /* // like-sign - >2 track FD comparison */
    /* // RATIO */
    /* pt.SetStdColors(kBlack,9); */
    /* pt.SetTextPos(1.48692, 66.7423); */
    /* pt.SetLegendPos(0.224066, 0.0471266, 0.556708, 0.276112); */
    /* pt.PlotRatio("fInvMass_FD_3plus", "LS (2 #sqrt{++ #times --} )"); */
    /* // ADD */
    /* pt.SetTextPos(1.48444, 77.0644); */
    /* pt.SetLegendPos(0.569156,0.548087,0.751037,0.687694); */
    /* pt.PlotAdd("fInvMass_FD_3plus", "LS (2 #sqrt{++ #times --} )"); */
    
    /* // 3+ template - >2 track FD comparison */
    /* // RATIO plot */
    /* pt.SetStdColors(kBlack,6); */
    /* pt.SetTextPos(1.46457, 64.7354); */
    /* pt.SetLegendPos(0.235131, 0.0618998, 0.417012, 0.248043); */
    /* pt.PlotRatio("fInvMass_FD_3plus", "fInvMass_3trks"); */
    /* pt.SetTextPos(1.46457,266.587); */
    /* // ADD plot */
    /* pt.SetLegendPos(0.238589,0.182006,0.42047,0.309204); */
    /* pt.SetYaxisRange(0.3, 1.2e3); */
    /* pt.PlotAdd("fInvMass_FD_3plus", "fInvMass_3trks"); */
    /* pt.PlotAdd("fInvMass_FD_3plus", "fInvMass_4trks"); */
    /* pt.PlotAdd("fInvMass_FD_3plus", "fInvMass_5trks"); */
    /* pt.SetYaxisRange(0.3, 4.2e3); */
    /* pt.SetTextPos(1.46457,743.401); */
    /* pt.PlotAdd("fInvMass_FD_3plus", "fInvMass_3plusTrks"); */
    
    /* // gamma-hit - has gammas FD comparison */
    /* // RATIO plot */
    /* pt.SetStdColors(kBlack,8); */
    /* pt.SetTextPos(1.46458, 64.7354); */
    /* pt.SetLegendPos(0.235131, 0.0618998, 0.417012, 0.248043); */
    /* pt.PlotRatio("fInvMass_FD_hasGammas", "fInvMass_GammaDet_bg"); */
    /* pt.SetTextPos(1.46457,266.587); */
    /* // ADD plot */
    /* pt.SetTextPos(1.46458, 290.587); */
    /* pt.SetLegendPos(0.238589,0.182006,0.42047,0.309204); */
    /* pt.SetYaxisRange(0.3, 1.2e3); */
    /* pt.PlotAdd("fInvMass_FD_hasGammas", "fInvMass_GammaDet_bg"); */

    /* // combined background (gamma-hit + 3-tracks) */
    /* // create the background */
    /* pt.SetYaxisRange(0.3, 2.e3); */
    /* pt.CreateHistWithScale("fInvMass_GammaDet_bg", "fInvMass_3trks", 9.8, 0.04, */ 
    /*         "BG template (#gamma-hit + 3 track BG)"); */

    /* // RATIO */
    /* pt.SetStdColors(kBlack, kRed+1); */
    /* pt.PlotRatio("fInvMass_FD", "BG template (#gamma-hit + 3 track BG)"); */
    /* // ADD */
    /* pt.SetTextPos(1.45711,409.875); */
    /* pt.SetLegendPos(0.230982,0.239917,0.412863,0.368149); */
    /* pt.PlotAdd("fInvMass_FD", "BG template (#gamma-hit + 3 track BG)"); */

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
    pt_full.SetAxisRange(0.,5.);
    pt_full.SetSigBgColors(9);
    pt_full.SetTextPos(2.76802,47.8254);
    pt_full.SetXaxisText("Cluster distance to nearest track in #phi-#eta space");
    pt_full.SetYaxisText("Significance");
    pt_full.PlotSignificance("fdPhiEta_pion", "fdPhiEta_gamma");
    pt_full.ResetColors();
 
    pt_full.SetLog(kTRUE);
    pt_full.SetAxisRange(0.,1.3);
    pt_full.SetTextPos(0.711745,4137.76);
    pt_full.SetLegendPos(0.592669, 0.477413,0.827109,0.667351);
    pt_full.SetXaxisText("E (GeV)");
    pt_full.SetYaxisText("Counts / (0.03 GeV)");
    pt_full.PlotSigBg("fEnergy_SIG", "fEnergy_BG");
    /* pt_full.ResetTextPos(); */
    pt_full.ToggleDisplayCanvas();
    pt_full.PlotRatio("fEnergy_BG", "fEnergy_SIG");
    /* pt_full.ResetTextPos(); */
    /* pt_full.SetAxisRange(0.,5.); */
    /* pt_full.SetLegendPos(0.587137,0.552361,0.821577,0.709446); */
    /* pt_full.SetXaxisText("Cluster distance to nearest track in #phi-#eta space"); */
    /* pt_full.SetYaxisText("Counts"); */
    /* pt_full.PlotSigBg("fdPhiEta_pion", "fdPhiEta_gamma"); */
}

