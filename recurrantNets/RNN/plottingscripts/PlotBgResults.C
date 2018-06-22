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
#include <TList.h>
#include <TString.h>
#include <TFile.h>
/* #include <TRatioPlot.h> */

Bool_t plotLog = kTRUE;

TString Title(TH1F* hist)
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
    else if (title_str=="fInvMass_GammaDet_bg")  out_str = "#gamma hit BG (FD)";
    else if (title_str=="fInvMass_LS_plus")  out_str = "Like sign (+)";
    else if (title_str=="fInvMass_LS_minus") out_str = "Like sign (-)";
    else if (title_str=="fNb_trks_passed") out_str = "N_{tracks}";
    else out_str = hist->GetTitle();
    
    return out_str;
}

TH1F* ScaleHist(TH1F* h_toScale, TH1F* h_main)
{
    Double_t integral_toScale = h_toScale->Integral(1,h_toScale->GetSize()-2); 
    Double_t integral_main    = h_main->Integral(1,h_main->GetSize()-2); 
    Double_t scale = integral_main / integral_toScale;

    h_toScale->Scale(scale);
    return h_toScale;
}

TH1F* ScaleHist(TH1F* h_toScale, Double_t finalAUC)
{
    Double_t integral_toScale = h_toScale->Integral(1,h_toScale->GetSize()-2); 
    Double_t scale = finalAUC / integral_toScale;

    h_toScale->Scale(scale);
    return h_toScale;
}

Double_t nextLogNb(Double_t ymax)
{
    return std::pow(10., std::ceil(std::log10(ymax)));
}

TCanvas* rp(TH1F* main_hist, TH1F* h2, TH1F* h3=0x0, TH1F* h4=0x0, TH1F* h5=0x0) 
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
    main_hist->SetAxisRange(0., 2.5,"X");
    if (plotLog) pad1->SetLogy();               // pad1 becomes the current pad
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
    if (plotLog) axis = new TGaxis(xmin, std::pow(10., ymin), xmin, std::pow(10.,ymax), std::pow(10., ymin), std::pow(10.,ymax), 505,"G");
    else axis = new TGaxis(xmin, ymin, xmin, ymax, ymin, ymax, 510,"");
    // ChangeLable only works since root-v6.07
    axis->ChangeLabel(1,-1,-1,-1,-1,-1," ");
    axis->SetLabelFont(43); // Absolute font size in pixel (precision 3)
    axis->SetLabelSize(37);
    axis->Draw();

    // lower plot will be in pad
    c->cd();          // Go back to the main canvas before defining pad2
    TPad *pad2 = new TPad("pad2", "pad2", 0, 0.05, 1, 0.3);
    pad2->SetTopMargin(0);
    pad2->SetBottomMargin(0.35);
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
    main_hist->GetYaxis()->SetTitleSize(39);
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
    h_diff12->GetYaxis()->SetTitleSize(39);
    h_diff12->GetYaxis()->SetTitleFont(43);
    h_diff12->GetYaxis()->SetTitleOffset(1.3);
    h_diff12->GetYaxis()->SetLabelFont(43); // Absolute font size in pixel (precision 3)
    h_diff12->GetYaxis()->SetLabelSize(35);
    /* h_diff12->GetYaxis()->SetLabelOffset(0.008); */
    h_diff12->GetXaxis()->SetLabelOffset(0.017);

    // X axis ratio plot settings
    h_diff12->GetXaxis()->SetTitleSize(39);
    h_diff12->GetXaxis()->SetTitleFont(43);
    h_diff12->GetXaxis()->SetTitleOffset(4.);
    h_diff12->GetXaxis()->SetLabelFont(43); // Absolute font size in pixel (precision 3)
    h_diff12->GetXaxis()->SetLabelSize(35);

    pad1->cd();
    pad1->Update();
    TLatex tex;
    tex.SetTextFont(43);
    tex.SetTextSize(37);
    TString tex_str = "#splitline{#splitline{ALICE simulation, this thesis}{Pythia-8 MBR (#varepsilon=0.08)}}{#sqrt{s}=13 TeV}";
    if (plotLog) tex.DrawLatex(0.55*xmax, 0.23*std::pow(10.,ymax), tex_str);
    else tex.DrawLatex(0.55*xmax, 0.81*ymax,tex_str);

    TLegend* leg = 0x0;
    if (plotLog) leg = new TLegend(0.224541,0.13638,0.556761,0.364879);
    else leg = new TLegend(0.599332, 0.269521, 0.932387, 0.543001);
    leg->AddEntry(main_hist, (Title(main_hist)).Data(), "le");
    leg->AddEntry(h2, (Title(h2)).Data(), "pe");
    if (h3) leg->AddEntry(h3, (Title(h3)).Data(), "pe");
    if (h4) leg->AddEntry(h4, (Title(h4)).Data(), "pe");
    if (h5) leg->AddEntry(h5, (Title(h5)).Data(), "pe");
    leg->SetTextSizePixels(34);
    leg->SetFillStyle(0);
    leg->Draw();

    c->Update();
    return c;
}

TCanvas* PlotAddHists(TH1F* hist1, TH1F* h_2=0x0, TH1F* h_3=0x0, TH1F* h_4=0x0)
{
    // go into batch mode where canvases are not drawn:

    TCanvas* canv = new TCanvas("InvMass","",1450,1000);

    Double_t integral = hist1->Integral(1,hist1->GetSize()-2); 
    Double_t integral_2(-1), integral_3(-1), integral_4(-1);
    if (h_2) integral_2 = h_2->Integral(1,h_2->GetSize()-2); 
    if (h_3) integral_3 = h_3->Integral(1,h_3->GetSize()-2); 
    if (h_4) integral_4 = h_4->Integral(1,h_4->GetSize()-2); 

    TString n_1, n_2, n_3, n_4;
    n_1.Form("%i", Int_t(integral));
    n_2.Form("%3.2f", 100.*Double_t(integral_2)/Double_t(integral));
    n_3.Form("%3.2f", 100.*Double_t(integral_3)/Double_t(integral));
    n_4.Form("%3.2f", 100.*Double_t(integral_4)/Double_t(integral));

    hist1->Sumw2();
    if (h_2) h_2->Sumw2();
    if (h_3) h_3->Sumw2();
    if (h_4) h_4->Sumw2();

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
    hist1->GetYaxis()->SetTitleSize(39);
    hist1->GetXaxis()->SetTitleSize(39);
    hist1->GetXaxis()->SetTitleFont(43);
    hist1->GetYaxis()->SetTitleFont(43);
    hist1->GetXaxis()->SetLabelSize(35);
    hist1->GetYaxis()->SetLabelSize(35);
    hist1->GetXaxis()->SetLabelFont(43);
    hist1->GetYaxis()->SetLabelFont(43);

    hist1->SetAxisRange(0., 2.5,"X");

    hist1->Draw();
    if (plotLog) gPad->SetLogy();

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

    canv->Update();

    TLatex tex;
    tex.SetTextFont(43);
    tex.SetTextSize(37);
    Double_t xmax, ymax;
    xmax = canv->GetFrame()->GetX2();
    ymax = canv->GetFrame()->GetY2();
    printf("xmax: %.2f, y_max: %.2f\n", xmax, ymax);
    TString tex_str = "#splitline{#splitline{ALICE simulation, this thesis}{Pythia-8 MBR (#varepsilon=0.08)}}{#sqrt{s}=13 TeV}";
    if (plotLog) tex.DrawLatex(0.550*xmax, 0.23*std::pow(10.,ymax), tex_str);
    else tex.DrawLatex(0.55*xmax, 0.81*ymax, tex_str);


    TLegend* leg = 0x0; 
    if (plotLog) leg = new TLegend(0.21616,0.180698, 0.450276, 0.389117);
    else leg = new TLegend(0.615331, 0.413758, 0.861188, 0.659138);
    leg->AddEntry(hist1, (Title(hist1)).Data(), "pe");
    if (h_2) leg->AddEntry(h_2, (Title(h_2) + "(" + n_2 + "%)").Data(), "pe");
    if (h_3) leg->AddEntry(h_3, (Title(h_3) + "(" + n_3 + "%)").Data(), "pe");
    if (h_4) leg->AddEntry(h_4, (Title(h_4) + "(" + n_4 + "%)").Data(), "pe");
    leg->SetTextSizePixels(34);
    leg->SetFillStyle(0);
    leg->Draw();

    canv->Update();
    return canv;
}

TCanvas* PlotHist(TH1F* hist)
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
    hist->GetYaxis()->SetTitleSize(39);
    hist->GetXaxis()->SetTitleSize(39);
    hist->GetXaxis()->SetTitleFont(43);
    hist->GetYaxis()->SetTitleFont(43);
    hist->GetXaxis()->SetLabelSize(35);
    hist->GetYaxis()->SetLabelSize(35);
    /* hist->GetXaxis()->SetLabelFont(42); */
    /* hist->GetYaxis()->SetLabelFont(42); */

    hist->SetAxisRange(0., 2.5,"X");

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

    canv->Update();
    return canv;
}


void SetStyle(Bool_t graypalette=kFALSE) 
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
    gStyle->SetLegendFont(42);

    printf("ALICE style set.\n");
}

void Plot(TString fname, TString hname1, TString hname2="", TString hname3="", TString hname4="", TString hname5="")
{
    /* gROOT->SetBatch(kTRUE); */
    // output has the same name (but with pdf ending)
    TString output_file = fname;
    // remove the .root at the end of the filename ".root" = 5 characters at the end
    output_file.Remove(output_file.Length()-5, 5);
    // now append .pdf
    output_file += ".pdf";

    // set the alice plot-style
    SetStyle();

    // open the file
    TFile* file = TFile::Open(fname.Data());
    if (!file) { printf("<E> File %s not found!\n", fname.Data()); gSystem->Exit(1);  }
    TDirectory* dir = file->GetDirectory("BGTask");
    TList* lst = (TList*)dir->Get("BGOutputContainer");
    if (!lst) { printf("<E> List not found in file %s\n", fname.Data()); gSystem->Exit(1);  }
    // create a plotting canvas

    // 1: loop through one of the files to get all hists
    TH1F* h_1 = (TH1F*)lst->FindObject(hname1);
    TH1F *h_2(0x0), *h_3(0x0), *h_4(0x0), *h_5(0x0);
    /* hist->Add((TH1F*)file->Get(hname1)); */
    if (hname2!="") h_2 = (TH1F*)lst->FindObject(hname2);
    if (hname3!="") h_3 = (TH1F*)lst->FindObject(hname3);
    if (hname4!="") h_4 = (TH1F*)lst->FindObject(hname4);
    if (hname5!="") h_5 = (TH1F*)lst->FindObject(hname5);
    if (!h_1 || !h_1->InheritsFrom("TH1")) { printf("<E> No histogram found named %s\n", hname1.Data()); gSystem->Exit(1);  }
    if (h_2 && !h_2->InheritsFrom("TH1"))  { printf("<E> No histogram found named %s\n", hname2.Data()); gSystem->Exit(1);  }
    if (h_3 && !h_3->InheritsFrom("TH1"))  { printf("<E> No histogram found named %s\n", hname3.Data()); gSystem->Exit(1);  }
    if (h_4 && !h_4->InheritsFrom("TH1"))  { printf("<E> No histogram found named %s\n", hname4.Data()); gSystem->Exit(1);  }
    if (h_5 && !h_5->InheritsFrom("TH1"))  { printf("<E> No histogram found named %s\n", hname5.Data()); gSystem->Exit(1);  }

    /* Double_t integral_1 = h_1->Integral(1,h_1->GetSize()-2); */ 
    /* Double_t integral_2(-1), integral_3(-1), integral_4(-1); */
    /* if (h_2) integral_2 = h_2->Integral(1,h_2->GetSize()-2); */ 
    /* if (h_3) integral_3 = h_3->Integral(1,h_3->GetSize()-2); */ 
    /* if (h_4) integral_4 = h_4->Integral(1,h_4->GetSize()-2); */ 

    /* TString n_1, n_2, n_3, n_4; */
    /* n_1.Form("%i", Int_t(integral_1)); */
    /* n_2.Form("%i", Int_t(integral_2)); */
    /* n_3.Form("%i", Int_t(integral_3)); */
    /* n_4.Form("%i", Int_t(integral_4)); */

    /* h_1->Sumw2(); */
    /* if (h_2) h_2->Sumw2(); */
    /* if (h_3) h_3->Sumw2(); */
    /* if (h_4) h_4->Sumw2(); */

    TCanvas *c = 0x0;
    c = rp(h_1, h_2, h_3, h_4, h_5);
    /* if (h_1 && !h_2 && !h_3 && !h_4) c = PlotHist(h_1); */
    /* if (h_2 || h_3 || h_4) c = PlotAddHists(h_1, h_2, h_3, h_4); */

    c->SaveAs(output_file.Data());
    printf("Saving output in %s\n", output_file.Data());
}
 

