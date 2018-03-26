/// ~~~{.C}

#include <iostream>
#include <string>
#include <stdlib.h> 
#include <sys/stat.h>
#include <time.h>

#include <TROOT.h>
#include <TSystem.h>
#include <TDirectory.h>
#include <TSystemDirectory.h>
#include <TSystemFile.h>
#include <TList.h>
#include <TString.h>
#include <TFile.h>
#include <TChain.h>
#include <TTree.h>

void InvMassPlot(TString input_dirname, TString path_to_cepfilter_macro, TString output_prefix="", 
        Int_t filter=-1, TString path_to_evt_id_txt="")
{
    // first we check if there is a file behind the path_to_evt_id_txt
    Bool_t evts_from_file = kFALSE;
    std::vector<UInt_t> evt_id_vec;
    evt_id_vec.clear();
    if (path_to_evt_id_txt.EndsWith(".txt")) {
        evts_from_file = kTRUE;
        std::ifstream file(path_to_evt_id_txt.Data());
        Int_t inp_int;
    	while (file >> inp_int) evt_id_vec.push_back(inp_int);
    }
    if (evt_id_vec.size() == 0 && evt_from_file) gSystem->Exit(0);

    // get the cut-selection info
    gROOT->ProcessLine((".L " + path_to_cepfilter_macro).Data());

    // file extensions
    const char *file_ext = ".root";
    
    // tree and branch name
    TString tree_name = "CEP";
    TString cep_evts_branchname = "CEPEvents";
    TString cep_raw_evts_branchname = "CEPRawEvents";

    if (file_addon_str == "") file_addon_str = ".root";
    else {
        file_addon_str.Insert(0, "_");
        file_addon_str += ".root";
    }
         
    TSystemDirectory input_dir(input_dirname, input_dirname);
    TList *input_files = input_dir.GetListOfFiles();

    TChain *cep_tree = new TChain(tree_name);
    if(input_files) {
        std::cout << "\nReading \"*" << file_ext << "\" files from \"" << input_dirname << "\"..." << std::endl;
        TSystemFile *file;
        TString fname;
        TIter next(input_files);
        while((file = (TSystemFile*)next())) {
            fname = file->GetName();
            if(!file->IsDirectory() && fname.EndsWith(file_ext)) {
                cep_tree->Add(input_dirname + fname);
            }
        }
    }

    CEPRawEventBuffer* cep_raw_evt = 0x0;
    CEPEventBuffer* cep_evt = 0x0;

    cep_tree->SetBranchAddress(cep_evts_branchname,     &cep_evt);
    cep_tree->SetBranchAddress(cep_raw_evts_branchname, &cep_raw_evt);

    TFile* outfile=new TFile((output_prefix+"invar_mass_plot"+file_addon_str).Data(), "RECREATE");

    // event level features:
    TH1F* hInvarmass = new TH1F("invar_mass", "", 100, 0, 10);

    // LHC16-filter
    Int_t cu;
    Bool_t isDG, isNDG;
    Int_t nseltracks, n_tracks;
    Int_t lhc16_filter = -999;
    // for display purposes only!
    Float_t progress = 0.0;
    Int_t barWidth = 70;
    /////////////////////////////
    // Number of evts in TChain
    UInt_t nevts = cep_tree->GetEntries();
    UInt_t event_nb(0);
    if (!evts_from_file) evt_id_vec.resize(nevts,0);
    for (UInt_t ii(0); ii<evt_id_vec.size(); ii++)
    {
        // display purposes only ////////////////////
        progress = float(ii-evt_offset)/float(n_evts_to_read);
        std::cout << "[";
        Int_t pos = barWidth * progress;
        for (Int_t i(0); i < barWidth; ++i) {
            if (i < pos) std::cout << "=";
            else if (i == pos) std::cout << ">";
            else std::cout << " ";
        }
        std::cout << "] " << int(progress * 100.0) << " %\r";
        std::cout.flush();
        // display purposes only ////////////////////

        event_nb = ii;
        if (evts_from_file) event_nb = evt_id_vec[ii];

        cep_tree->GetEntry(event_nb);
        if (!cep_evt) {
            std::cout << "Event number " << ii << " cannot be found!" << std::endl;
            gSystem->Exit(1);
        }

        /* cu: 
         *      ein bitword mit dem man den Filter steuern kann, für Simulationen ist ein
         *      Wert von 74 geeignet.
         *
         * isDG: 
         *      wird dir am Ende sagen, ob das event ev ein CEP Kandidat ist
         *
         * nseltracks:
         *      ist dann die Anzahl tracks im event 
         */ 
        n_tracks = 2;
        if (!evts_from_file){
            if (filter==0)      { cu=74; mode=0; }    // !V0
            else if (filter==1) { cu=65539; mode=0 }  // checkSPD hard
            else if (filter==2) { cu=65539; mode=1 }  // maxdnclu=maxdnfchips=maxdnfFOchips=1e5
            else if (filter==3) { cu=65539; mode=2 }  // maxnSingle=1
            else if (filter==4) { cu=65539; mode=3 }  // maxnSingle=2
            else                { cu=104; mode=0;}    // !AD
            nseltracks = LHC16Filter(cep_evt,kFALSE,cu,isDG,isNDG); 
            if (isDG==kFALSE || nseltracks!=n_tracks) continue;
        }
       
        // initialize charge_sum with 0 for every new event
        Int_t evt_charge_sum_var = 0;
        
        // HL track info
        CEPTrackBuffer* trk = 0x0;
        TVector3 v;
        UInt_t hlt_kk(0);
        TLorentzVector lor_vec;  // initialized by (0.,0.,0.,0.)
        for (UInt_t kk(0); kk<cep_evt->GetnTracks(); kk++)
        {
            trk = cep_evt->GetTrack(kk);
            if (!trk) break;
            // momentum 
            v = trk->GetMomentum(); 
            lor_vec.SetPtEtaPhiM(v.Pt(),v.Eta(),v.Phi(),trk->GetMCMass());
            // Bayes
            /* trk->GetPIDBayesStatus(); */
            /* trk->GetPIDBayesProbability(AliPID::kPion); */
            /* trk->GetPIDBayesProbability(AliPID::kKaon); */
            /* trk->GetPIDBayesProbability(AliPID::kProton); */
            /* trk->GetPIDBayesProbability(AliPID::kElectron); */
            /* trk->GetPIDBayesProbability(AliPID::kMuon); */

            // charge sign
            evt_charge_sum_var += trk->GetChargeSign();
        }
        if (evt_charge_sum_var!=0) continue;
        hInvarmass->Fill(
    }
    // cursor of status display has to move to the next line
    std::cout << std::endl;
    std::cout << "100%:   conversion finished " << std::endl;
    ///////////////////////

    outfile->cd();
    hInvarmass->Write();
    outfile->Close();
    delete outfile;

    std::cout << "\nSaving the TList in " << (output_prefix+"feature_plots"+file_addon_str).Data() << std::endl;
}
