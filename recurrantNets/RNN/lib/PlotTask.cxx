/// ~~~{.C}

#include <iostream>
#include <stdlib.h> 
#include <sys/stat.h>
#include <time.h>

#include <TROOT.h>
#include <TSystem.h>
#include <TObjArray.h>
#include <TDirectory.h>
#include <TSystemDirectory.h>
#include <TSystemFile.h>
#include <TString.h>
#include <TFile.h>
#include <TCanvas.h>
#include <TPad.h>
#include <TLatex.h>
#include <TLegend.h>
#include <TStyle.h>
#include <TFrame.h>
#include <TGaxis.h>
#include <TMath.h>

#include "PlotTask.h"

ClassImp(PlotTask)

// ____________________________________________________________________________
PlotTask::PlotTask(TString fname, TString option)
  : fLogPlot(kFALSE)  
  , fSetBatch(kTRUE)  
  , fHistList (0x0)
  , fOutFileBaseName (0x0)
  , fTitleSize(42)
  , fLableSize(36)
  , fPlotTextSize(39)
  , fLegendTextSize(35)
  , fAxisMin(0.)
  , fAxisMax(2.5)
  , fLegXmin(-999.)
  , fLegXmax(-999.)
  , fLegYmin(-999.)
  , fLegYmax(-999.)
{
    fOutFileBaseName = fname;
    // remove the .root at the end of the filename ".root" = 5 characters at the end
    fOutFileBaseName.Remove(fOutFileBaseName.Length()-5, 5);

    // open the file
    TFile* file = TFile::Open(fname.Data());
    if (!file) { printf("<E> File %s not found!\n", fname.Data()); gSystem->Exit(1);  }
    TDirectory* dir = 0x0;
    if (option.Contains("BG")) {
        dir = file->GetDirectory("BGTask");
        if (!dir) { 
            printf("<E> Direcotry not found in file %s\n", fname.Data()); 
            gSystem->Exit(1); 
        }
        fHistList = (TList*)dir->Get("BGOutputContainer");
    }
    else {
        dir = file->GetDirectory("EMCALTask");
        if (!dir) { 
            printf("<E> Direcotry not found in file %s\n", fname.Data()); 
            gSystem->Exit(1); 
        }
        fHistList = (TList*)dir->Get("EMCALOutputContainer");
    }
    if (!fHistList) { 
        printf("<E> List not found in file %s\n", fname.Data()); 
        gSystem->Exit(1); 
    }
    fHistList->SetOwner(kTRUE);
    // close the file, is not needed any more 
    file->Close();
    if (file) { delete file; file=0x0; dir=0x0; }
}

//_______________________________________________________________________________________
PlotTask::~PlotTask()
{
    delete fHistList;
    fHistList = 0x0;
}

//_______________________________________________________________________________________
void PlotTask::ResetSizes()
{
    fTitleSize = 42;
    fLableSize= 36;
    fPlotTextSize = 39;
    fLegendTextSize = 35;

    fAxisMax = 2.5;
    fAxisMin = 0.;
}

//_______________________________________________________________________________________
void PlotTask::ResetLegendPos()
{
    fLegXmin = -999; fLegXmax = -999; fLegYmin = -999; fLegYmax = -999;
}

//_______________________________________________________________________________________
void PlotTask::SetLegendPos(Double_t xmin, Double_t ymin, Double_t xmax, Double_t ymax)
{
    fLegXmin = xmin;
    fLegXmax = xmax;
    fLegYmin = ymin;
    fLegYmax = ymax;
}

//_______________________________________________________________________________________
void PlotTask::RelativeTextPosition(Double_t &x, Double_t &y) const
{
    if (fPlotTextSize>42) { x=0.47; y = (fLogPlot) ? 0.20 : 0.76; }
    if (fPlotTextSize==43) { x=0.49; y = (fLogPlot) ? 0.21 : 0.77; }
    if (fPlotTextSize==42) { x=0.51; y = (fLogPlot) ? 0.22 : 0.78; }
    if (fPlotTextSize==41) { x=0.52; y = (fLogPlot) ? 0.22 : 0.79; }
    if (fPlotTextSize==40) { x=0.53; y = (fLogPlot) ? 0.22 : 0.79; }
    if (fPlotTextSize==39) { x=0.54; y = (fLogPlot) ? 0.22 : 0.80; }
    if (fPlotTextSize==38) { x=0.56; y = (fLogPlot) ? 0.23 : 0.81; }
    if (fPlotTextSize==37) { x=0.57; y = (fLogPlot) ? 0.23 : 0.82; }
    if (fPlotTextSize<37) { x=0.58; y = (fLogPlot) ? 0.24 : 0.82; }
}

//_______________________________________________________________________________________
TString PlotTask::Title(TH1F* hist) const
{
    TString out_str;
    TString title_str = hist->GetTitle();
    if (title_str=="fInvMass_FD") out_str = "Feed down";
    else if (title_str=="fInvMass_FD_emcal") out_str = "only #gamma";
    else if (title_str=="fInvMass_FD_3plus") out_str = "3+ tracks";
    else if (title_str=="fInvMass_FD_other") out_str = "other";
    else if (title_str=="fInvMass_FD_hasGammas") out_str = "FD has #gamma";
    else if (title_str=="fInvMass_3trks") out_str = "3 track BG";
    else if (title_str=="fInvMass_4trks") out_str = "4 track BG";
    else if (title_str=="fInvMass_5trks") out_str = "5 track BG";
    else if (title_str=="fInvMass_6trks") out_str = "6 track BG";
    else if (title_str=="fInvMass_3plusTrks") out_str = "3-10 track BG";
    else if (title_str=="fInvMass_GammaDet_sig") out_str = "#gamma hit BG (sig)";
    else if (title_str=="fInvMass_GammaDet_bg")  out_str = "#gamma hit BG";
    else if (title_str=="fInvMass_LS_plus")  out_str = "Like sign (+)";
    else if (title_str=="fInvMass_LS_minus") out_str = "Like sign (-)";
    else if (title_str=="fNb_trks_passed") out_str = "N_{tracks}";
    // plots from the emcal task
    else if (title_str=="fGammaE") out_str = "Primary E_{#gamma}";
    else if (title_str=="fSecondaryE_SIG") out_str = "Secondary E_{#gamma} (sig)";
    else if (title_str=="fSecondaryE_BG")  out_str = "Secondary E_{#gamma} (FD)";
    else if (title_str=="fEnergy_SIG") out_str = "Cluster energy (sig)";
    else if (title_str=="fEnergy_BG")  out_str = "Cluster energy (FD)";
    else if (title_str=="fdPhiEta_pion") out_str = "Pion #phi-#eta distance";
    else if (title_str=="fdPhiEta_gamma") out_str = "Gamma #phi-#eta distance";
    else out_str = hist->GetTitle();
    
    return out_str;
}

//_______________________________________________________________________________________
TCanvas* PlotTask::SigBg(TH1F* h_sig, TH1F* h_bg) const
{
    // go into batch mode where canvases are not drawn:

    TCanvas* canv = new TCanvas("SigBg","",1450,1000);

    /* Double_t integral_sig = h_sig->Integral(1,h_sig->GetSize()-2); */ 
    /* Double_t integral_bg  = h_bg->Integral( 1,h_bg->GetSize()-2); */ 

    h_sig->Sumw2();
    h_bg->Sumw2();

    // histogram specific actions
    h_sig->SetLineColor(kGreen+1);
    h_sig->SetLineWidth(2);
    h_sig->SetMarkerStyle(20);
    h_sig->SetMarkerSize(1.4);
    h_sig->SetMarkerColor(kGreen+1);

    /* axis labels have to be set seperately, */
    /* gStyle is somehow set to default for the next adjustments */
    h_sig->GetXaxis()->SetTitleOffset(1.1);
    h_sig->GetYaxis()->SetTitleOffset(1.15);
    h_sig->GetYaxis()->SetTitleSize(fTitleSize);
    h_sig->GetXaxis()->SetTitleSize(fTitleSize);
    h_sig->GetXaxis()->SetTitleFont(43);
    h_sig->GetYaxis()->SetTitleFont(43);
    h_sig->GetXaxis()->SetLabelSize(fLableSize);
    h_sig->GetYaxis()->SetLabelSize(fLableSize);
    h_sig->GetXaxis()->SetLabelFont(43);
    h_sig->GetYaxis()->SetLabelFont(43);

    h_bbg->GetXaxis()->SetTitleOffset(1.1);
    h_bbg->GetYaxis()->SetTitleOffset(1.15);
    h_bbg->GetYaxis()->SetTitleSize(fTitleSize);
    h_bbg->GetXaxis()->SetTitleSize(fTitleSize);
    h_bbg->GetXaxis()->SetTitleFont(43);
    h_bbg->GetYaxis()->SetTitleFont(43);
    h_bbg->GetXaxis()->SetLabelSize(fLableSize);
    h_bbg->GetYaxis()->SetLabelSize(fLableSize);
    h_bbg->GetXaxis()->SetLabelFont(43);
    h_bbg->GetYaxis()->SetLabelFont(43);


    if (h_sig->GetBinContent(h_sig->GetMaximumBin())>=h_bg->GetBinContent(h_bg->GetMaximumBin())){
        // h-sig has to be plotted first as to not crop away any hist points
        h_sig->GetXaxis()->SetTitle("m_{#pi#pi} (GeV/c^{2})");
        h_sig->GetYaxis()->SetTitle("Counts / (0.03 GeV/c^{2})");
        h_sig->SetAxisRange(fAxisMin, fAxisMax,"X");
        h_sig->Draw();
        h_bg->Draw("same");
    } else {
        h_bg->GetXaxis()->SetTitle("m_{#pi#pi} (GeV/c^{2})");
        h_bg->GetYaxis()->SetTitle("Counts / (0.03 GeV/c^{2})");
        h_bg->SetAxisRange(fAxisMin, fAxisMax,"X");
        h_bg->Draw();
        h_sig->Draw("same");
    }

    if (fLogPlot) gPad->SetLogy();

    h_bg->SetLineColor(kRed+1);
    h_bg->SetLineWidth(2);
    h_bg->SetMarkerStyle(4);
    h_bg->SetMarkerSize(1.4);
    h_bg->SetMarkerColor(kRed+1);

    canv->Update();

    TLatex tex;
    tex.SetTextFont(43);
    tex.SetTextSize(fPlotTextSize);
    Double_t xmax, ymax;
    xmax = canv->GetFrame()->GetX2();
    ymax = canv->GetFrame()->GetY2();
    printf("xmax: %.2f, y_max: %.2f\n", xmax, ymax);
    TString tex_str = "#splitline{#splitline{ALICE simulation, this thesis}{Pythia-8 MBR (#varepsilon=0.08)}}{#sqrt{s}=13 TeV}";
    Double_t x(0), y(0);
    RelativeTextPosition(x,y);
    if (fLogPlot) { y -= 0.02; tex.DrawLatex(x*xmax, y*std::pow(10.,ymax), tex_str); }
    else { y += 0.02; tex.DrawLatex(x*xmax, y*ymax, tex_str); }

    TLegend* leg = 0x0; 
    // the legend can be set manually but only temporarily 
    // (fLegXmin was picked arbitrarily, all leg coordinates should be positive)
    if (fLegXmin<=0) {
        if (fLogPlot) leg = new TLegend(0.21616,0.180698, 0.450276, 0.389117);
        else leg = new TLegend(0.595994, 0.48152, 0.874309, 0.661191);
    } else leg = new TLegend(fLegXmin, fLegYmin, fLegXmax, fLegYmax);
    leg->AddEntry(h_sig, Title(h_sig).Data(), "pe");
    if (h_bg) leg->AddEntry(h_bg, Title(h_bg).Data(), "pe");
    leg->SetTextFont(43);
    leg->SetTextSize(fLegendTextSize);
    leg->SetFillStyle(0);
    leg->Draw();

    canv->Update();
    return canv;
}

//_______________________________________________________________________________________
void PlotTask::PlotSigBg(TString hnameSig, TString hnameBg, Bool_t kNorm) const
{
    gROOT->SetBatch(fSetBatch);
    // set the alice plot-style
    SetStyle();

    if (!fHistList->FindObject(hnameSig)) {
        printf("<E> No histogram found named %s\n", hnameSig.Data()); return; }
    TH1F* h_sig = (TH1F*)((TH1F*)fHistList->FindObject(hnameSig))->Clone((hnameSig+"_cln").Data());
    // 2nd histogram
    if (!fHistList->FindObject(hnameBg)){ 
        printf("<E> No histogram found named %s\n", hnameBg.Data()); return; }
    TH1F* h_bg = (TH1F*)((TH1F*)fHistList->FindObject(hnameBg))->Clone((hnameBg+"_cln").Data());

    if (kNorm) h_bg = ScaleHist(h_bg, h_sig);

    TCanvas *c = 0x0;
    c = SigBg(h_sig, h_bg);

    TString outstr = fOutFileBaseName + "_" + hnameSig + "_" + hnameBg;
    if (fLogPlot) outstr += "_log";
    c->SaveAs((outstr+"_SigBg.pdf").Data());
    if (fSetBatch) delete c;
    c = 0x0;
    printf("Saving output in %s\n", (outstr+"_SigBg.pdf").Data());
}

//_______________________________________________________________________________________
void PlotTask::AddHists(TString finalName, TString hname1, TString hname2,
                        TString hname3, TString hname4)
{
    TH1F* h_1 = (TH1F*)((TH1F*)fHistList->FindObject(hname1))->Clone(finalName);
    h_1->SetTitle(finalName);
    TH1F* h_2 = 
        (TH1F*)((TH1F*)fHistList->FindObject(hname2))->Clone((hname2+"_cln").Data());

    TH1F *h_3(0x0), *h_4(0x0);
    if (hname3!="") h_3 = 
        (TH1F*)((TH1F*)fHistList->FindObject(hname3))->Clone((hname3+"_cln").Data());
    if (hname4!="") h_4 = 
        (TH1F*)((TH1F*)fHistList->FindObject(hname4))->Clone((hname4+"_cln").Data());

    h_1->Add(h_2);
    if(h_3) h_1->Add(h_3);
    if(h_4) h_1->Add(h_4);

    h_1->Sumw2();

    fHistList->Add(h_1);
}

//_______________________________________________________________________________________
TH1F* PlotTask::ScaleHist(TH1F* h_toScale, TH1F* h_main) const
{
    Double_t integral_toScale = h_toScale->Integral(1,h_toScale->GetSize()-2); 
    Double_t integral_main    = h_main->Integral(1,h_main->GetSize()-2); 
    Double_t scale = integral_main / integral_toScale;

    h_toScale->Scale(scale);
    return h_toScale;
}

//_______________________________________________________________________________________
TH1F* PlotTask::ScaleHist(TH1F* h_toScale, Double_t finalAUC) const
{
    Double_t integral_toScale = h_toScale->Integral(1,h_toScale->GetSize()-2); 
    Double_t scale = finalAUC / integral_toScale;

    h_toScale->Scale(scale);
    return h_toScale;
}

//_______________________________________________________________________________________
TCanvas* PlotTask::rp(TH1F* main_hist, TH1F* h2, TH1F* h3, TH1F* h4, TH1F* h5) const
{
   // Define the Canvas
    TCanvas *c = new TCanvas("c", "canvas", 1450, 1000);

    // Upper plot will be in pad1
    TPad *pad1 = new TPad("pad1", "pad1", 0, 0.3, 1, 1.0);
    pad1->SetBottomMargin(0); // Upper and lower plot are joined
    pad1->Draw();             // Draw the upper pad: pad1
    pad1->cd();               // pad1 becomes the current pad
    main_hist->SetStats(0);          // No statistics on upper plot
    main_hist->Draw();                   // Draw main_hist
    h2 = ScaleHist(h2, main_hist);
    h2->Draw("same ep");                 // Draw h2 on top of main_hist
    if (h3) {
        h3 = ScaleHist(h3, main_hist);
        h3->Draw("same ep");         // Draw h2 on top of main_hist
    }
    if (h4) {
        h4 = ScaleHist(h4, main_hist);
        h4->Draw("same ep");         // Draw h2 on top of main_hist
    }
    if (h5) {
        h5 = ScaleHist(h5, main_hist);
        h5->Draw("same ep");         // Draw h2 on top of main_hist
    }
    main_hist->SetAxisRange(fAxisMin, fAxisMax,"X");
    if (fLogPlot) pad1->SetLogy();               // pad1 becomes the current pad
    pad1->Update();

    Double_t xmin, xmax, ymin, ymax;
    xmin = pad1->GetFrame()->GetX1();
    xmax = pad1->GetFrame()->GetX2();
    ymin = pad1->GetFrame()->GetY1();
    ymax = pad1->GetFrame()->GetY2();
    printf("xmin: %.2f, xmin: %.2f, ymin: %.2f, ymax: %.2f\n", xmin, xmax, ymin, ymax);
    // Do not draw the Y axis label on the upper plot and redraw a small
    // axis instead, in order to avoid the first label (0) to be clipped.
    main_hist->GetYaxis()->SetLabelSize(0.);
    
    TGaxis *axis = 0x0; 
    if (fLogPlot) axis = new TGaxis(xmin, std::pow(10., ymin), xmin, std::pow(10.,ymax), std::pow(10., ymin), std::pow(10.,ymax), 505,"G");
    else axis = new TGaxis(xmin, ymin, xmin, ymax, ymin, ymax, 510,"");
    // ChangeLable only works since root-v6.07
    /* if ((Int_t)TMath::Floor(gROOT->GetVersionInt()/TMath::Power(10.,TMath::Floor(TMath::Log10(gROOT->GetVersionInt()))))==6) axis->ChangeLabel(1,-1,-1,-1,-1,-1," "); */
    axis->SetLabelFont(43); // Absolute font size in pixel (precision 3)
    axis->SetLabelSize(fLableSize);
    axis->Draw();

    // lower plot will be in pad
    c->cd();          // Go back to the main canvas before defining pad2
    TPad *pad2 = new TPad("pad2", "pad2", 0, 0.05, 1, 0.3);
    pad2->SetTopMargin(0);
    pad2->SetBottomMargin(0.4);
    pad2->SetGridy(); // vertical grid
    pad2->Draw();
    pad2->cd();       // pad2 becomes the current pad

    // Define the ratio plot
    TH1F *h_diff12 = (TH1F*)main_hist->Clone("h_diff12");
    h_diff12->SetMinimum(0.);  // Define Y ..
    h_diff12->SetMaximum(2.3); // .. range
    h_diff12->Sumw2();
    h_diff12->SetStats(0);      // No statistics on lower plot
    h_diff12->Divide(h2);
    h_diff12->SetMarkerStyle(21);
    h_diff12->SetMarkerSize(1.2);
    h_diff12->SetMarkerColor(8);
    h_diff12->SetLineColor(8);
    h_diff12->Draw("ep");       // Draw the ratio plot

    // Define the second ratio plot
    if (h3) {
        TH1F *h_diff13 = (TH1F*)main_hist->Clone("h_diff13");
        h_diff13->Sumw2();
        h_diff13->Divide(h3);
        h_diff13->SetMarkerStyle(25);
        h_diff13->SetMarkerSize(1.2);
        h_diff13->SetMarkerColor(6);
        h_diff13->SetLineColor(6);
        h_diff13->Draw("same ep");       // Draw the ratio plot
    }
    // Define the second ratio plot
    if (h4) {
        TH1F *h_diff14 = (TH1F*)main_hist->Clone("h_diff14");
        h_diff14->Sumw2();
        h_diff14->Divide(h4);
        h_diff14->SetMarkerStyle(20);
        h_diff14->SetMarkerSize(1.2);
        h_diff14->SetMarkerColor(kBlue);
        h_diff14->SetLineColor(kBlue);
        h_diff14->Draw("same ep");       // Draw the ratio plot
    }
    if (h5) {
        TH1F *h_diff15 = (TH1F*)main_hist->Clone("h_diff15");
        h_diff15->Sumw2();
        h_diff15->Divide(h5);
        h_diff15->SetMarkerStyle(24);
        h_diff15->SetMarkerSize(1.2);
        h_diff15->SetMarkerColor(kRed);
        h_diff15->SetLineColor(kRed);
        h_diff15->Draw("same ep");       // Draw the ratio plot
    }

    // main_hist settings
    main_hist->SetLineColor(kBlack);
    main_hist->SetLineWidth(2);
    // Y axis main_hist plot settings
    /* main_hist->GetXaxis()->SetTitle("m_{#pi#pi} (GeV/c^{2})"); */
    main_hist->GetYaxis()->SetTitle("Counts / (0.03 GeV/c^{2})");
    main_hist->GetYaxis()->SetTitleSize(fTitleSize);
    main_hist->GetYaxis()->SetTitleFont(43);
    main_hist->GetYaxis()->SetTitleOffset(1.3);
    main_hist->GetYaxis()->SetLabelOffset(0.02);

    // h2 settings
    h2->SetLineColor(8);
    h2->SetMarkerStyle(21);
    h2->SetMarkerSize(1.2);
    h2->SetMarkerColor(8);
    h2->SetLineWidth(2);
    
    // h2 settings
    if (h3) {
        h3->SetLineColor(6);
        h3->SetMarkerStyle(25);
        h3->SetMarkerSize(1.2);
        h3->SetMarkerColor(6);
        h3->SetLineWidth(2);
    }
    if (h4) {
        h4->SetLineColor(kBlue);
        h4->SetMarkerStyle(20);
        h4->SetMarkerSize(1.2);
        h4->SetMarkerColor(kBlue);
        h4->SetLineWidth(2);
    }
    if (h5) {
        h5->SetLineColor(kRed);
        h5->SetMarkerStyle(24);
        h5->SetMarkerSize(1.2);
        h5->SetMarkerColor(kRed);
        h5->SetLineWidth(2);
    }      

    // Ratio plot (h_diff12) settings
    h_diff12->SetTitle(""); // Remove the ratio title

    // Y axis ratio plot settings
    h_diff12->GetYaxis()->SetTitle("ratios");
    h_diff12->GetXaxis()->SetTitle("m_{#pi#pi} (GeV/c^{2})");
    h_diff12->GetYaxis()->SetNdivisions(303);
    h_diff12->GetYaxis()->SetTitleSize(fTitleSize);
    h_diff12->GetYaxis()->SetTitleFont(43);
    h_diff12->GetYaxis()->SetTitleOffset(1.3);
    h_diff12->GetYaxis()->SetLabelFont(43); // Absolute font size in pixel (precision 3)
    h_diff12->GetYaxis()->SetLabelSize(fLableSize);
    /* h_diff12->GetYaxis()->SetLabelOffset(0.008); */
    h_diff12->GetXaxis()->SetLabelOffset(0.017);

    // X axis ratio plot settings
    h_diff12->GetXaxis()->SetTitleSize(fTitleSize);
    h_diff12->GetXaxis()->SetTitleFont(43);
    h_diff12->GetXaxis()->SetTitleOffset(4.);
    h_diff12->GetXaxis()->SetLabelFont(43); // Absolute font size in pixel (precision 3)
    h_diff12->GetXaxis()->SetLabelSize(fLableSize);

    pad1->cd();
    pad1->Update();
    TLatex tex;
    tex.SetTextFont(43);
    tex.SetTextSize(fPlotTextSize);
    TString tex_str = "#splitline{#splitline{ALICE simulation, this thesis}{Pythia-8 MBR (#varepsilon=0.08)}}{#sqrt{s}=13 TeV}";
    Double_t x(0), y(0);
    RelativeTextPosition(x,y);
    if (fLogPlot) tex.DrawLatex(x*xmax, y*std::pow(10.,ymax), tex_str);
    else tex.DrawLatex(x*xmax, y*ymax, tex_str);

    TLegend* leg = 0x0;
    if (fLegXmin<=0) {
        if (fLogPlot) leg = new TLegend(0.224541,0.13638,0.556761,0.364879);
        else leg = new TLegend(0.620166, 0.295981, 0.898481, 0.590789);
    } else leg = new TLegend(fLegXmin, fLegYmin, fLegXmax, fLegYmax);
    leg->AddEntry(main_hist, (Title(main_hist)).Data(), "le");
    leg->AddEntry(h2, (Title(h2)).Data(), "pe");
    if (h3) leg->AddEntry(h3, (Title(h3)).Data(), "pe");
    if (h4) leg->AddEntry(h4, (Title(h4)).Data(), "pe");
    if (h5) leg->AddEntry(h5, (Title(h5)).Data(), "pe");
    leg->SetTextFont(43);
    leg->SetTextSize(fLegendTextSize);
    leg->SetFillStyle(0);
    leg->Draw();
    // if legend was set temporararily we undo these changes to not disturb the next plot

    c->Update();
    return c;
}

//_______________________________________________________________________________________
TCanvas* PlotTask::PlotAddHists(TH1F* hist1, TH1F* h_2, TH1F* h_3, TH1F* h_4, TH1F* h_5) const
{
    // go into batch mode where canvases are not drawn:

    TCanvas* canv = new TCanvas("InvMass","",1450,1000);

    Double_t integral = hist1->Integral(1,hist1->GetSize()-2); 
    Double_t integral_2(-1), integral_3(-1), integral_4(-1), integral_5(-1);
    if (h_2) integral_2 = h_2->Integral(1,h_2->GetSize()-2); 
    if (h_3) integral_3 = h_3->Integral(1,h_3->GetSize()-2); 
    if (h_4) integral_4 = h_4->Integral(1,h_4->GetSize()-2); 
    if (h_5) integral_5 = h_5->Integral(1,h_5->GetSize()-2); 

    TString n_1, n_2, n_3, n_4, n_5;
    n_1.Form("%i", Int_t(integral));
    n_2.Form("%3.2f", 100.*Double_t(integral_2)/Double_t(integral));
    n_3.Form("%3.2f", 100.*Double_t(integral_3)/Double_t(integral));
    n_4.Form("%3.2f", 100.*Double_t(integral_4)/Double_t(integral));
    n_5.Form("%3.2f", 100.*Double_t(integral_5)/Double_t(integral));

    hist1->Sumw2();
    if (h_2) h_2->Sumw2();
    if (h_3) h_3->Sumw2();
    if (h_4) h_4->Sumw2();
    if (h_5) h_5->Sumw2();

    // histogram specific actions
    hist1->SetLineColor(kBlack);
    hist1->SetLineWidth(2);
    hist1->SetMarkerStyle(20);
    hist1->SetMarkerSize(1.4);
    hist1->SetMarkerColor(kBlack);

    hist1->GetXaxis()->SetTitle("m_{#pi#pi} (GeV/c^{2})");
    hist1->GetYaxis()->SetTitle("Counts / (0.03 GeV/c^{2})");
    /* axis labels have to be set seperately, */
    /* gStyle is somehow set to default for the next adjustments */
    hist1->GetXaxis()->SetTitleOffset(1.1);
    hist1->GetYaxis()->SetTitleOffset(1.15);
    hist1->GetYaxis()->SetTitleSize(fTitleSize);
    hist1->GetXaxis()->SetTitleSize(fTitleSize);
    hist1->GetXaxis()->SetTitleFont(43);
    hist1->GetYaxis()->SetTitleFont(43);
    hist1->GetXaxis()->SetLabelSize(fLableSize);
    hist1->GetYaxis()->SetLabelSize(fLableSize);
    hist1->GetXaxis()->SetLabelFont(43);
    hist1->GetYaxis()->SetLabelFont(43);

    hist1->SetAxisRange(fAxisMin, fAxisMax,"X");

    hist1->Draw();
    if (fLogPlot) gPad->SetLogy();

    if (h_2) {
        // 6 = magenta
        h_2->SetLineColor(6);
        h_2->SetLineWidth(2);
        h_2->SetMarkerStyle(24);
        h_2->SetMarkerSize(1.4);
        h_2->SetMarkerColor(6);
        h_2->Draw("same");
    }
    if (h_3) {
        // 8 = dark green
        h_3->SetLineColor(8);
        h_3->SetLineWidth(2);
        // 21 = filled square
        h_3->SetMarkerStyle(21);
        h_3->SetMarkerSize(1.4);
        h_3->SetMarkerColor(8);
        h_3->Draw("same");
    }
    if (h_4) {
        h_4->SetLineColor(kBlue);
        h_4->SetLineWidth(2);
        h_4->SetMarkerStyle(25);
        h_4->SetMarkerSize(1.4);
        h_4->SetMarkerColor(kBlue);
        h_4->Draw("same");
    }
    if (h_5) {
        h_5->SetLineColor(kYellow);
        h_5->SetLineWidth(2);
        h_5->SetMarkerStyle(27);
        h_5->SetMarkerSize(1.4);
        h_5->SetMarkerColor(kYellow);
        h_5->Draw("same");
    }
    canv->Update();

    TLatex tex;
    tex.SetTextFont(43);
    tex.SetTextSize(fPlotTextSize);
    Double_t xmax, ymax;
    xmax = canv->GetFrame()->GetX2();
    ymax = canv->GetFrame()->GetY2();
    printf("xmax: %.2f, y_max: %.2f\n", xmax, ymax);
    TString tex_str = "#splitline{#splitline{ALICE simulation, this thesis}{Pythia-8 MBR (#varepsilon=0.08)}}{#sqrt{s}=13 TeV}";
    Double_t x(0), y(0);
    RelativeTextPosition(x,y);
    y += 0.02;
    if (fLogPlot) tex.DrawLatex(x*xmax, y*std::pow(10.,ymax), tex_str);
    else tex.DrawLatex(x*xmax, y*ymax, tex_str);


    TLegend* leg = 0x0; 
    if (fLegXmin<=0) {
        if (fLogPlot) leg = new TLegend(0.21616,0.180698, 0.450276, 0.389117);
        else leg = new TLegend(0.595994, 0.48152, 0.874309, 0.661191);
    } else leg = new TLegend(fLegXmin, fLegYmin, fLegXmax, fLegYmax);
    leg->AddEntry(hist1, (Title(hist1)).Data(), "pe");
    if (h_2) leg->AddEntry(h_2, (Title(h_2) + "(" + n_2 + "%)").Data(), "pe");
    if (h_3) leg->AddEntry(h_3, (Title(h_3) + "(" + n_3 + "%)").Data(), "pe");
    if (h_4) leg->AddEntry(h_4, (Title(h_4) + "(" + n_4 + "%)").Data(), "pe");
    if (h_5) leg->AddEntry(h_5, (Title(h_5) + "(" + n_5 + "%)").Data(), "pe");
    leg->SetTextFont(43);
    leg->SetTextSize(fLegendTextSize);
    leg->SetFillStyle(0);
    leg->Draw();

    canv->Update();
    return canv;
}

//_______________________________________________________________________________________
TCanvas* PlotTask::PlotHist(TH1F* hist) const
{
    TCanvas* canv = new TCanvas("InvMass","",1450,1000);

    Double_t integral = hist->Integral(1,hist->GetSize()-2); 
    TString n;
    n.Form("%i", Int_t(integral));

    hist->Sumw2();

    hist->SetLineColor(kBlue);
    hist->SetLineWidth(2);
    hist->SetMarkerStyle(20);
    hist->SetMarkerSize(1.4);
    hist->SetMarkerColor(kBlue);

    hist->GetXaxis()->SetTitle("m_{#pi#pi} (GeV/c^{2})");
    hist->GetYaxis()->SetTitle("Counts / (0.03 GeV/c^{2})");
    // axis labels have to be set seperately,
    // gStyle is somehow set to default for the next adjustments
    hist->GetXaxis()->SetTitleOffset(1.1);
    hist->GetYaxis()->SetTitleOffset(1.15);
    hist->GetYaxis()->SetTitleSize(fTitleSize);
    hist->GetXaxis()->SetTitleSize(fTitleSize);
    hist->GetXaxis()->SetTitleFont(43);
    hist->GetYaxis()->SetTitleFont(43);
    hist->GetXaxis()->SetLabelSize(fLableSize);
    hist->GetYaxis()->SetLabelSize(fLableSize);
    /* hist->GetXaxis()->SetLabelFont(42); */
    /* hist->GetYaxis()->SetLabelFont(42); */

    hist->SetAxisRange(fAxisMin, fAxisMax,"X");

    hist->Draw();
    /* hist->SetMarkerStyle(k); */
    /* hist->SetMarkerSize(1.2); */
    /* hist->SetMarkerColor(kBlack); */
    TLatex tex;
    tex.DrawLatex(1.11086, 622.392, "#splitline{#splitline{ALICE simulation, this work}{Pythia-8 MBR (#varepsilon=0.08)}}{#sqrt{s}=13 TeV}");

    TLegend* leg = new TLegend(0.575377, 0.522727, 0.908291, 0.613636);
    leg->AddEntry(hist, (Title(hist)).Data(), "pe");
    /* leg->SetTextSize(0.04); */
    leg->SetFillStyle(0);
    leg->Draw();
    // if legend was set temporararily we undo these changes to not disturb the next plot

    canv->Update();
    return canv;
}

//_______________________________________________________________________________________
void PlotTask::SetStyle(Bool_t graypalette) const 
{
    gStyle->Reset("Plain");
    gStyle->SetOptTitle(0);
    gStyle->SetOptStat(0);
    if(graypalette) gStyle->SetPalette(8,0);
    else gStyle->SetPalette(57);
    gStyle->SetCanvasColor(10);
    gStyle->SetCanvasBorderMode(0);
    gStyle->SetFrameLineWidth(1.5);
    gStyle->SetFrameFillColor(kWhite);
    gStyle->SetPadColor(10);
    gStyle->SetPadTickX(1);
    gStyle->SetPadTickY(1);
    gStyle->SetPadBottomMargin(0.14);
    gStyle->SetPadLeftMargin(0.14);
    gStyle->SetHistLineWidth(2);
    /* gStyle->SetHistLineColor(kRed); */
    gStyle->SetFuncWidth(2);
    gStyle->SetFuncColor(kGreen);
    gStyle->SetLineWidth(2);
    gStyle->SetMarkerStyle(20);
    gStyle->SetMarkerSize(1);
    gStyle->SetLabelSize(0.045,"xyz");
    gStyle->SetLabelOffset(0.015,"y");
    gStyle->SetLabelOffset(0.015,"x");
    gStyle->SetLabelColor(kBlack,"xyz");
    gStyle->SetTitleSize(0.06,"xy");
    gStyle->SetTitleOffset(1.25,"y");
    gStyle->SetTitleOffset(1.2,"x");
    /* gStyle->SetTitleFillColor(kWhite); */
    gStyle->SetTextSizePixels(24);
    gStyle->SetTextFont(42);
    gStyle->SetTickLength(0.04,"X"); gStyle->SetTickLength(0.04,"Y");
    gStyle->SetLegendBorderSize(0);
    gStyle->SetLegendFillColor(kWhite);
    //  gStyle->SetFillColor(kWhite);
    gStyle->SetLegendFont(43);

    printf("ALICE style set.\n");
}

//_______________________________________________________________________________________
void PlotTask::PlotRatio(TString hname1, TString hname2, TString hname3, 
                         TString hname4, TString hname5) const 
{
    gROOT->SetBatch(fSetBatch);
    // set the alice plot-style
    SetStyle();

    if (!fHistList->FindObject(hname1)) {
        printf("<E> No histogram found named %s\n", hname1.Data()); return; }
    TH1F* h_1 = (TH1F*)((TH1F*)fHistList->FindObject(hname1))->Clone((hname1+"_cln").Data());
    // 2nd histogram
    if (!fHistList->FindObject(hname2)){ 
        printf("<E> No histogram found named %s\n", hname2.Data()); return; }
    TH1F* h_2 = (TH1F*)((TH1F*)fHistList->FindObject(hname2))->Clone((hname2+"_cln").Data());

    TH1F *h_3(0x0), *h_4(0x0), *h_5(0x0);
    // 3rd histogram
    if (hname3!="" && !fHistList->FindObject(hname3)){ 
        printf("<E> No histogram found named %s\n", hname3.Data()); return; }
    if (hname3!="") h_3 = 
        (TH1F*)((TH1F*)fHistList->FindObject(hname3))->Clone((hname3+"_cln").Data());
    // 4th histogram
    if (hname4!="" && !fHistList->FindObject(hname4)){ 
        printf("<E> No histogram found named %s\n", hname4.Data()); return; }
    if (hname4!="") h_4 = 
        (TH1F*)((TH1F*)fHistList->FindObject(hname4))->Clone((hname4+"_cln").Data());
    // 5th histogram
    if (hname5!="" && !fHistList->FindObject(hname5)){ 
        printf("<E> No histogram found named %s\n", hname5.Data()); return; }
    if (hname5!="") h_5 = 
        (TH1F*)((TH1F*)fHistList->FindObject(hname5))->Clone((hname5+"_cln").Data());    

    TCanvas *c = 0x0;
    c = rp(h_1, h_2, h_3, h_4, h_5);

    TString outstr = fOutFileBaseName + "_" + hname1;
    if (hname2!="") outstr += "_" + hname2;
    if (hname3!="") outstr += "_" + hname3;
    if (hname4!="") outstr += "_" + hname4;
    if (hname5!="") outstr += "_" + hname5;
    if (fLogPlot) outstr += "_log";
    c->SaveAs((outstr+"_ratio.pdf").Data());
    if (fSetBatch) delete c;
    c = 0x0;
    printf("Saving output in %s\n", (outstr+"_ratio.pdf").Data());
}

//_______________________________________________________________________________________
void PlotTask::PlotAdd(TString hname1, TString hname2, 
                       TString hname3, TString hname4, TString hname5) const
{
    gROOT->SetBatch(fSetBatch);
    // set the alice plot-style
    SetStyle();

    // we clone the histograms from the fHistList in order not to mess up the original
    // histograms (that in term would violate the const functions
    if (!fHistList->FindObject(hname1)) {
        printf("<E> No histogram found named %s\n", hname1.Data()); return; }
    TH1F* h_1 = (TH1F*)((TH1F*)fHistList->FindObject(hname1))->Clone((hname1+"_cln").Data());
    // 2nd histogram
    if (!fHistList->FindObject(hname2)){ 
        printf("<E> No histogram found named %s\n", hname2.Data()); return; }
    TH1F* h_2 = (TH1F*)((TH1F*)fHistList->FindObject(hname2))->Clone((hname2+"_cln").Data());

    TH1F *h_3(0x0), *h_4(0x0), *h_5(0x0);
    // 3rd histogram
    if (hname3!="" && !fHistList->FindObject(hname3)){ 
        printf("<E> No histogram found named %s\n", hname3.Data()); return; }
    if (hname3!="") h_3 = 
        (TH1F*)((TH1F*)fHistList->FindObject(hname3))->Clone((hname3+"_cln").Data());
    // 4th histogram
    if (hname4!="" && !fHistList->FindObject(hname4)){ 
        printf("<E> No histogram found named %s\n", hname4.Data()); return; }
    if (hname4!="") h_4 = 
        (TH1F*)((TH1F*)fHistList->FindObject(hname4))->Clone((hname4+"_cln").Data());
    // 5th histogram
    if (hname5!="" && !fHistList->FindObject(hname5)){ 
        printf("<E> No histogram found named %s\n", hname5.Data()); return; }
    if (hname5!="") h_5 = 
        (TH1F*)((TH1F*)fHistList->FindObject(hname5))->Clone((hname5+"_cln").Data());    

    TCanvas *c = 0x0;
    if (h_1 && !h_2 && !h_3 && !h_4) c = PlotHist(h_1);
    if (h_2 || h_3 || h_4) c = PlotAddHists(h_1, h_2, h_3, h_4, h_5);

    TString outstr = fOutFileBaseName + "_" + hname1;
    if (hname2!="") outstr += "_" + hname2;
    if (hname3!="") outstr += "_" + hname3;
    if (hname4!="") outstr += "_" + hname4;
    if (hname5!="") outstr += "_" + hname5;
    if (fLogPlot) outstr += "_log";
    c->SaveAs((outstr+"_add.pdf").Data());
    if (fSetBatch) delete c;
    c = 0x0;
    printf("Saving output in %s\n", (outstr+"_add.pdf").Data());
}
