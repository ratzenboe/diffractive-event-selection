#include "TString.h"

#include "PlotTask.h"

void PlotMCResults(TString fname_taskres)
{
    // We assume the files to add are in the same directory as the fname_taskres, if not
    // move the files in that directory
    std::string curr_dir(fname_taskres);
    // find last occurance of "/" (determining the full path to the directory)
    std::size_t path_end_idx = curr_dir.find_last_of("/\\");
    // take only the substring to the last occurance (+1 to also get the slash /)
    TString dir((curr_dir.substr(0,path_end_idx+1)).c_str());

    PlotTask pt(fname_taskres);
    pt.ChangeTitleOfHist("invar_mass_full_recon", "Signal");
    pt.ChangeTitleOfHist("invar_mass_feed_down", "Feed down");
    pt.AddFile(dir+"all_features_invar_mass_plot.root", "all_feats");
    pt.AddFile("blf_invar_mass_plot.root", "blf");
    pt.AddFile("blf_phieta_bayes_invar_mass_plot.root", "blf_phieta_bayes");
    pt.AddFile("blf_phieta_bayes_nsigma_invar_mass_plot.root", "blf_phieta_bayes_nsigma");
    pt.AddFile("blf_phieta_invar_mass_plot.root", "blf_phieta");
    pt.AddFile("blf_phieta_pt_opang_invar_mass_plot.root", "blf_phieta_pt_opang");
    pt.AddFile("blf_pt_invar_mass_plot.root", "blf_pt");
    // calculate bg reduction rate
    pt.ReductionRateHist("invar_mass_feed_down", "invar_mass_feed_down_all_feats",               "fd_reduction_all_feats",               "All features");
    pt.ReductionRateHist("invar_mass_feed_down", "invar_mass_feed_down_blf_phieta_bayes",        "fd_reduction_blf_phieta_bayes",        "BLF_{#phi#eta} + Bayes");
    pt.ReductionRateHist("invar_mass_feed_down", "invar_mass_feed_down_blf_phieta_bayes_nsigma", "fd_reduction_blf_phieta_bayes_nsigma", "BLF_{#phi#eta} + Bayes + N-#sigma");
    pt.ReductionRateHist("invar_mass_feed_down", "invar_mass_feed_down_blf_phieta",              "fd_reduction_blf_phieta",              "BLF_{#phi#eta}");
    pt.ReductionRateHist("invar_mass_feed_down", "invar_mass_feed_down_blf_phieta_pt_opang",     "fd_reduction_blf_phieta_pt_opang",     "BLF_{#phi#eta} + p_{T} + #varphi_{1-2}");
    pt.ReductionRateHist("invar_mass_feed_down", "invar_mass_feed_down_blf_pt",                  "fd_reduction_blf_pt",                  "BLF + p_{T}");
    pt.ReductionRateHist("invar_mass_feed_down", "invar_mass_feed_down_blf",                     "fd_reduction_blf",                     "BLF");
    // calculate signal efficiency
    pt.SignalEfficiency("invar_mass_full_recon", "invar_mass_full_recon_all_feats",               "sig_eff_all_feats",               "All features");
    pt.SignalEfficiency("invar_mass_full_recon", "invar_mass_full_recon_blf_phieta_bayes",        "sig_eff_blf_phieta_bayes",        "BLF_{#phi#eta} + Bayes");
    pt.SignalEfficiency("invar_mass_full_recon", "invar_mass_full_recon_blf_phieta_bayes_nsigma", "sig_eff_blf_phieta_bayes_nsigma", "BLF_{#phi#eta} + Bayes + N-#sigma");
    pt.SignalEfficiency("invar_mass_full_recon", "invar_mass_full_recon_blf_phieta",              "sig_eff_blf_phieta",              "BLF_{#phi#eta}");
    pt.SignalEfficiency("invar_mass_full_recon", "invar_mass_full_recon_blf_phieta_pt_opang",     "sig_eff_blf_phieta_pt_opang",     "BLF_{#phi#eta} + p_{T} + #varphi_{1-2}");
    pt.SignalEfficiency("invar_mass_full_recon", "invar_mass_full_recon_blf_pt",                  "sig_eff_blf_pt",                  "BLF + p_{T}");
    pt.SignalEfficiency("invar_mass_full_recon", "invar_mass_full_recon_blf",                     "sig_eff_blf",                     "BLF");

    // change title bg
    pt.ChangeTitleOfHist("invar_mass_feed_down_all_feats", "All features");
    pt.ChangeTitleOfHist("invar_mass_feed_down_blf", "BLF");
    pt.ChangeTitleOfHist("invar_mass_feed_down_blf_phieta", "BLF_{#phi#eta}");
    pt.ChangeTitleOfHist("invar_mass_feed_down_blf_phieta_bayes", "BLF_{#phi#eta} + Bayes");
    pt.ChangeTitleOfHist("invar_mass_feed_down_blf_phieta_bayes_nsigma", "BLF_{#phi#eta} + Bayes + N-#sigma");
    pt.ChangeTitleOfHist("invar_mass_feed_down_blf_pt", "BLF + p_{T}");
    pt.ChangeTitleOfHist("invar_mass_feed_down_blf_phieta_pt_opang", "BLF_{#phi#eta} + p_{T} + #varphi_{1-2}");
    // change title signal
    pt.ChangeTitleOfHist("invar_mass_full_recon_all_feats", "All features");
    pt.ChangeTitleOfHist("invar_mass_full_recon_blf", "BLF");
    pt.ChangeTitleOfHist("invar_mass_full_recon_blf_phieta", "BLF_{#phi#eta}");
    pt.ChangeTitleOfHist("invar_mass_full_recon_blf_phieta_bayes", "BLF_{#phi#eta} + Bayes");
    pt.ChangeTitleOfHist("invar_mass_full_recon_blf_phieta_bayes_nsigma", "BLF_{#phi#eta} + Bayes + N-#sigma");
    pt.ChangeTitleOfHist("invar_mass_full_recon_blf_pt", "BLF + p_{T}");
    pt.ChangeTitleOfHist("invar_mass_full_recon_blf_phieta_pt_opang", "BLF_{#phi#eta} + p_{T} + #varphi_{1-2}");

    // plot signal vs backgroud
    pt.SetYaxisRange(3.*1e-1, 1.5*1e3);
    pt.SetLog(kTRUE);
    pt.SetAxisRange(0.,2.7);
    pt.SetTextPos(1.48195, 337.662);
    pt.SetLegendPos(0.176349, 0.353183, 0.368603,0.508214);
    pt.ChangeTitleOfHist("invar_mass_full_recon", "Signal");
    pt.PlotSigBg("invar_mass_full_recon", "invar_mass_feed_down");
    // plot reduced sig bg
    pt.ChangeTitleOfHist("invar_mass_feed_down_all_feats", "Feed down");
    pt.ChangeTitleOfHist("invar_mass_feed_down_blf", "Feed down");
    pt.ChangeTitleOfHist("invar_mass_feed_down_blf_phieta", "Feed down");
    pt.ChangeTitleOfHist("invar_mass_feed_down_blf_phieta_bayes", "Feed down");
    pt.ChangeTitleOfHist("invar_mass_feed_down_blf_phieta_bayes_nsigma", "Feed down");
    pt.ChangeTitleOfHist("invar_mass_feed_down_blf_pt", "Feed down");
    pt.ChangeTitleOfHist("invar_mass_feed_down_blf_phieta_pt_opang", "Feed down");
    // change title signal
    pt.ChangeTitleOfHist("invar_mass_full_recon_all_feats", "Signal");
    pt.ChangeTitleOfHist("invar_mass_full_recon_blf", "Signal");
    pt.ChangeTitleOfHist("invar_mass_full_recon_blf_phieta", "Signal");
    pt.ChangeTitleOfHist("invar_mass_full_recon_blf_phieta_bayes", "Signal");
    pt.ChangeTitleOfHist("invar_mass_full_recon_blf_phieta_bayes_nsigma", "Signal");
    pt.ChangeTitleOfHist("invar_mass_full_recon_blf_pt", "Signal");
    pt.ChangeTitleOfHist("invar_mass_full_recon_blf_phieta_pt_opang", "Signal");

    pt.SetLegendPos(0.17704,0.708419,0.368603, 0.86345);
    pt.SetLegHeader("All features");
    pt.PlotSigBg("invar_mass_full_recon_all_feats",               "invar_mass_feed_down_all_feats");

    pt.SetLegHeader("BLF_{#phi#eta} + Bayes");
    pt.PlotSigBg("invar_mass_full_recon_blf_phieta_bayes",        "invar_mass_feed_down_blf_phieta_bayes");

    pt.SetLegHeader("BLF_{#eta#phi} + Bayes + N-#sigma");
    pt.PlotSigBg("invar_mass_full_recon_blf_phieta_bayes_nsigma", "invar_mass_feed_down_blf_phieta_bayes_nsigma");

    pt.SetLegHeader("BLF_{#eta#phi}");
    pt.PlotSigBg("invar_mass_full_recon_blf_phieta",              "invar_mass_feed_down_blf_phieta");

    pt.SetLegHeader("BLT_{#eta#phi} + p_{T} + #varphi_{1-2}");
    pt.PlotSigBg("invar_mass_full_recon_blf_phieta_pt_opang",     "invar_mass_feed_down_blf_phieta_pt_opang");

    pt.SetLegHeader("BLF + p_{T}");
    pt.PlotSigBg("invar_mass_full_recon_blf_pt",                  "invar_mass_feed_down_blf_pt");   

    pt.SetLegHeader("BLF");
    pt.PlotSigBg("invar_mass_full_recon_blf",                     "invar_mass_feed_down_blf");                
    
    // reduced feed down plots
    pt.ChangeTitleOfHist("invar_mass_feed_down_all_feats", "All features");
    pt.ChangeTitleOfHist("invar_mass_feed_down_blf", "BLF");
    pt.ChangeTitleOfHist("invar_mass_feed_down_blf_phieta", "BLF_{#phi#eta}");
    pt.ChangeTitleOfHist("invar_mass_feed_down_blf_phieta_bayes", "BLF_{#phi#eta} + Bayes");
    pt.ChangeTitleOfHist("invar_mass_feed_down_blf_phieta_bayes_nsigma", "BLF_{#phi#eta} + Bayes + N-#sigma");
    pt.ChangeTitleOfHist("invar_mass_feed_down_blf_pt", "BLF + p_{T}");
    pt.ChangeTitleOfHist("invar_mass_feed_down_blf_phieta_pt_opang", "BLF_{#phi#eta} + p_{T} + #varphi_{1-2}");
    // change title signal
    pt.ChangeTitleOfHist("invar_mass_full_recon_all_feats", "All features");
    pt.ChangeTitleOfHist("invar_mass_full_recon_blf", "BLF");
    pt.ChangeTitleOfHist("invar_mass_full_recon_blf_phieta", "BLF_{#phi#eta}");
    pt.ChangeTitleOfHist("invar_mass_full_recon_blf_phieta_bayes", "BLF_{#phi#eta} + Bayes");
    pt.ChangeTitleOfHist("invar_mass_full_recon_blf_phieta_bayes_nsigma", "BLF_{#phi#eta} + Bayes + N-#sigma");
    pt.ChangeTitleOfHist("invar_mass_full_recon_blf_pt", "BLF + p_{T}");
    pt.ChangeTitleOfHist("invar_mass_full_recon_blf_phieta_pt_opang", "BLF_{#phi#eta} + p_{T} + #varphi_{1-2}");

    pt.ToggleShowPercent();
    pt.SetStdMarkers(1);
    pt.SetLegHeader("");

    pt.SetStdColors(kBlack, 6, 8, kBlue, kMagenta+2);
    pt.SetLegendPos(0.470263, 0.180698, 0.704703, 0.395277);
    pt.PlotAdd("invar_mass_feed_down", "invar_mass_feed_down_blf", "invar_mass_feed_down_blf_phieta", "invar_mass_feed_down_blf_phieta_bayes", "invar_mass_feed_down_blf_phieta_bayes_nsigma"); 
    pt.SetStdColors(kBlack, kAzure+1, kGreen+1, kRed);
    pt.SetLegendPos(0.498617, 0.174538, 0.733057, 0.389117);
    pt.PlotAdd("invar_mass_feed_down", "invar_mass_feed_down_blf_pt", "invar_mass_feed_down_blf_phieta_pt_opang", "invar_mass_feed_down_all_feats");
    pt.SetStdColors(kBlack, kAzure+1, kMagenta+2);
    pt.SetLegendPos(0.470263, 0.180698, 0.704703, 0.395277);
    pt.PlotAdd("invar_mass_feed_down", "invar_mass_feed_down_blf_pt", "invar_mass_feed_down_blf_phieta_bayes_nsigma");

    /* pt.ToggleDisplayCanvas(); */
    pt.ResetTextPos();
    pt.SetDrawOptionAdd("p");
    pt.SetLog(kFALSE);
    pt.SetYaxisRange(0., 1.05);
    pt.SetStdColors(6, 8, kBlue, kMagenta+2);
    pt.SetStdMarkers(24,21,25,33);
    /* pt.ToggleDisplayCanvas(); */
    pt.SetYaxisText("Bg reduction");
    pt.SetLegendPos(0.171508, 0.199179, 0.408022, 0.413758);
    pt.PlotAdd("fd_reduction_blf", "fd_reduction_blf_phieta", "fd_reduction_blf_phieta_bayes", "fd_reduction_blf_phieta_bayes_nsigma");
    pt.SetStdColors(6, kAzure+1, kGreen+1, kRed);
    pt.PlotAdd("fd_reduction_blf", "fd_reduction_blf_pt", "fd_reduction_blf_phieta_pt_opang", "fd_reduction_all_feats");
    pt.SetLegendPos(0.170816, 0.228953, 0.405256, 0.370637);
    pt.SetStdColors(kAzure+1, kMagenta+2);
    pt.PlotAdd("fd_reduction_blf_pt", "fd_reduction_blf_phieta_bayes_nsigma");

    // signal efficiency
    pt.SetTextPos(1.45711, 0.11864);
    pt.SetYaxisText("Signal efficiency");
    /* pt.SetLegendPos(0.156293, 0.200205, 0.391425, 0.341889); */
    pt.SetLegendPos(0.171508, 0.199179, 0.408022, 0.413758);
    pt.SetStdColors(6, 8, kBlue, kMagenta+2);
    pt.PlotAdd("sig_eff_blf", "sig_eff_blf_phieta", "sig_eff_blf_phieta_bayes", "sig_eff_blf_phieta_bayes_nsigma");
    pt.SetStdColors(6, kAzure+1, kGreen+1, kRed);
    pt.PlotAdd("sig_eff_blf", "sig_eff_blf_pt", "sig_eff_blf_phieta_pt_opang", "sig_eff_all_feats");
    /* pt.SetLegendPos(0.170816, 0.228953, 0.405256, 0.370637); */

    pt.SetStdColors(kAzure+1, kMagenta+2);
    pt.SetLegendPos(0.156293, 0.200205, 0.391425, 0.341889);
    pt.PlotAdd("sig_eff_blf_pt", "sig_eff_blf_phieta_bayes_nsigma");

    // plot signal eff and background reduction in one plot
    pt.ToggleDisplayCanvas();
    pt.SetYaxisText("Bg reduction & Signal efficiency");
    pt.SetStdColors(kRed, kGreen+1);
    pt.SetTextPos(1.45711, 0.11864);
    pt.SetLegendPos(0.156293, 0.200205, 0.391425, 0.341889);
    pt.SetLegHeader("All features");
    pt.ChangeTitleOfHist("sig_eff_all_feats", "Sig efficiency");
    pt.ChangeTitleOfHist("fd_reduction_all_feats", "Bg reduction");
    pt.PlotAdd("fd_reduction_all_feats", "sig_eff_all_feats");


}

void PlotKoalaResults(TString fname_taskres)
{
    // We assume the files to add are in the same directory as the fname_taskres, if not
    // move the files in that directory
    std::string curr_dir(fname_taskres);
    // find last occurance of "/" (determining the full path to the directory)
    std::size_t path_end_idx = curr_dir.find_last_of("/\\");
    // take only the substring to the last occurance (+1 to also get the slash /)
    TString dir((curr_dir.substr(0,path_end_idx+1)).c_str());

    PlotTask pt(fname_taskres);
    pt.ChangeTitleOfHist("invar_mass_full_recon", "Signal");
    pt.ChangeTitleOfHist("invar_mass_feed_down", "Feed down");
    // All featues and BLF featues from NN mode
    pt.AddFile(dir+"all_features_invar_mass_plot.root", "all_feats");
    pt.AddFile("blf_invar_mass_plot.root", "blf");
    // koala mode
    pt.AddFile("koala_blf_invar_mass_plot.root", "blf_koala");
    pt.AddFile("koala_all_feats_invar_mass_plot.root", "all_feats_koala");

    // calculate bg reduction rate
    pt.ReductionRateHist("invar_mass_feed_down", "invar_mass_feed_down_all_feats",       "fd_reduction_all_feats",               "All features");
    pt.ReductionRateHist("invar_mass_feed_down", "invar_mass_feed_down_blf",             "fd_reduction_blf",                     "BLF");
    pt.ReductionRateHist("invar_mass_feed_down", "invar_mass_feed_down_all_feats_koala", "fd_reduction_all_feats_koala",         "All features (CWoLa)");
    pt.ReductionRateHist("invar_mass_feed_down", "invar_mass_feed_down_blf_koala",       "fd_reduction_blf_koala",               "BLF (CWoLa)");
    // calculate signal efficiency
    pt.SignalEfficiency("invar_mass_full_recon", "invar_mass_full_recon_all_feats",               "sig_eff_all_feats",           "All features");
    pt.SignalEfficiency("invar_mass_full_recon", "invar_mass_full_recon_blf",                     "sig_eff_blf",                 "BLF");
    pt.SignalEfficiency("invar_mass_full_recon", "invar_mass_full_recon_all_feats_koala",         "sig_eff_all_feats_koala",     "All features (CWoLa)");
    pt.SignalEfficiency("invar_mass_full_recon", "invar_mass_full_recon_blf_koala",               "sig_eff_blf_koala",           "BLF (CWoLa)");

    // plot signal vs backgroud
    pt.SetYaxisRange(3.*1e-1, 1.5*1e3);
    pt.SetLog(kTRUE);
    pt.SetAxisRange(0.,2.7);
    pt.SetTextPos(1.48195, 337.662);
    pt.SetLegendPos(0.176349, 0.353183, 0.368603,0.508214);
    pt.ChangeTitleOfHist("invar_mass_full_recon", "Signal");
    pt.PlotSigBg("invar_mass_full_recon", "invar_mass_feed_down");
    
    // plot reduced sig bg
    pt.ChangeTitleOfHist("invar_mass_feed_down_blf_koala", "Feed down");
    pt.ChangeTitleOfHist("invar_mass_full_recon_blf_koala", "Signal");
    pt.SetLegendPos(0.17704,0.708419,0.368603, 0.86345);
    pt.SetLegHeader("BLF (CWoLa)");
    pt.PlotSigBg("invar_mass_full_recon_blf_koala", "invar_mass_feed_down_blf_koala");                

    // change title bg
    pt.ChangeTitleOfHist("invar_mass_feed_down_all_feats", "All features");
    pt.ChangeTitleOfHist("invar_mass_feed_down_blf", "BLF");
    pt.ChangeTitleOfHist("invar_mass_feed_down_all_feats_koala", "All features (CWoLa)");
    pt.ChangeTitleOfHist("invar_mass_feed_down_blf_koala", "BLF (CWoLa)");
    // change title signal
    pt.ChangeTitleOfHist("invar_mass_full_recon_all_feats", "All features");
    pt.ChangeTitleOfHist("invar_mass_full_recon_blf", "BLF");
    pt.ChangeTitleOfHist("invar_mass_full_recon_all_feats_koala", "All features (CWoLa)");
    pt.ChangeTitleOfHist("invar_mass_full_recon_blf_koala", "BLF (CWoLa)");

    pt.ToggleShowPercent();
    pt.SetStdMarkers(1);
    pt.SetLegHeader("");

    // plot reduced FD ivar-mass 
    pt.SetStdColors(kBlack, kGreen-3, kBlue-7);
    pt.SetLegendPos(0.470263, 0.180698, 0.704703, 0.395277);
    pt.PlotAdd("invar_mass_feed_down", "invar_mass_feed_down_blf_koala", "invar_mass_feed_down_all_feats_koala");

    /* pt.ToggleDisplayCanvas(); */
    // plot backgroudn reduction
    pt.ResetTextPos();
    pt.SetDrawOptionAdd("p");
    pt.SetLog(kFALSE);
    pt.SetYaxisRange(0., 1.05);
    pt.SetStdColors(6, kBlue-7);
    pt.SetStdMarkers(24,21,25,33);
    /* pt.ToggleDisplayCanvas(); */
    pt.SetYaxisText("Bg reduction");
    pt.SetLegendPos(0.171508, 0.199179, 0.408022, 0.413758);
    pt.PlotAdd("fd_reduction_blf", "fd_reduction_blf_koala");
    pt.SetStdColors(kRed, kBlue-7);
    pt.SetLegendPos(0.170816, 0.228953, 0.405256, 0.370637);
    pt.PlotAdd("fd_reduction_all_feats", "fd_reduction_all_feats_koala");
    
    // signal efficiency
    pt.SetTextPos(1.45711, 0.11864);
    pt.SetYaxisText("Signal efficiency");
    /* pt.SetLegendPos(0.156293, 0.200205, 0.391425, 0.341889); */
    pt.SetLegendPos(0.171508, 0.199179, 0.408022, 0.413758);
    pt.SetStdColors(6, kBlue-7);
    pt.PlotAdd("sig_eff_blf", "sig_eff_blf_koala");
    pt.SetStdColors(kRed, kGreen-3);
    pt.PlotAdd("sig_eff_all_feats", "sig_eff_all_feats_koala");

/*     pt.SetStdColors(kAzure+1, kMagenta+2); */
/*     pt.SetLegendPos(0.156293, 0.200205, 0.391425, 0.341889); */
/*     pt.PlotAdd("sig_eff_blf_pt", "sig_eff_blf_phieta_bayes_nsigma"); */

/*     // plot signal eff and background reduction in one plot */
/*     /1* pt.ToggleDisplayCanvas(); *1/ */
/*     pt.SetYaxisText("Bg reduction & Signal efficiency"); */
/*     pt.SetStdColors(kRed, kGreen+1); */
/*     pt.SetTextPos(1.45711, 0.11864); */
/*     pt.SetLegendPos(0.156293, 0.200205, 0.391425, 0.341889); */
/*     pt.SetLegHeader("All features"); */
/*     pt.ChangeTitleOfHist("sig_eff_all_feats", "Sig efficiency"); */
/*     pt.ChangeTitleOfHist("fd_reduction_all_feats", "Bg reduction"); */
/*     pt.PlotAdd("fd_reduction_all_feats", "sig_eff_all_feats"); */


}
