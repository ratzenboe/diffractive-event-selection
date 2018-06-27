/// ~~~{.C}

#include <iostream>
#include <string>
#include <stdlib.h> 
#include <sys/stat.h>
#include <time.h>
#include <algorithm>    // std::random_shuffle
#include <vector>   

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

void PlotInvMass(TString input_dirname, TString output_prefix="", TString path_to_evt_id_txt="")
{
    // first we check if there is a file behind the path_to_evt_id_txt
    std::vector<Int_t> evt_id_vec;
    evt_id_vec.clear();
    if (path_to_evt_id_txt.EndsWith(".txt")) {
        evts_from_file = kTRUE;
        std::ifstream txtfile(path_to_evt_id_txt.Data());
        Int_t inp_int;
    	while (txtfile >> inp_int) evt_id_vec.push_back(inp_int);
    }
    if (evt_id_vec.size() == 0) gSystem->Exit(0);
    if (evt_id_vec.size()>0) std::cout << evt_id_vec.size() << " events in txt file" << std::endl;

    else output_prefix = "ML_"+output_prefix;
    // file extensions
    const char *file_ext = ".root";
    
    // tree and branch name
    TString tree_name = "CEP";
    TString cep_evts_branchname = "CEPEvents";
    TString cep_raw_evts_branchname = "CEPRawEvents";

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
    } else { printf("<E> No files found in %s\n", input_dirname.Data()); gSystem->Exit(1); }

    CEPRawEventBuffer* cep_raw_evt = 0x0;
    CEPEventBuffer* cep_evt = 0x0;

    cep_tree->SetBranchAddress(cep_evts_branchname,     &cep_evt);
    cep_tree->SetBranchAddress(cep_raw_evts_branchname, &cep_raw_evt);

    TFile* outfile=new TFile((output_prefix+"invar_mass_plot.root").Data(), "RECREATE");

    // event level features:
    TH1F* hInvarmass = new TH1F("invar_mass_full_recon", "", 100, 0, 3.);
    TH1F* hInvarmass_fd = new TH1F("invar_mass_feed_down", "", 100, 0, 3.);
    TList* hist_list = new TList();

    // for display purposes only!
    Float_t progress = 0.0;
    Int_t barWidth = 70;
    /////////////////////////////
    // Number of evts in TChain
    UInt_t nevts = cep_tree->GetEntries();
    UInt_t event_nb, track_nb;
    if (!evts_from_file) evt_id_vec.resize(nevts,0);
    for (UInt_t ii(0); ii<evt_id_vec.size(); ii++)
    {
        // display purposes only ////////////////////
        /* progress = float(ii)/float(evt_id_vec.size()); */
        progress = float(ii)/float(evt_id_vec.size());
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
            std::cout << "Event number " << event_nb << " cannot be found!" << std::endl;
            gSystem->Exit(1);
        }

        // initialize charge_sum with 0 for every new event
        Bool_t kAllGood = kTRUE;
        
        // HL track info
        CEPTrackBuffer* trk = 0x0;
        TLorentzVector lor_vec;  // initialized by (0.,0.,0.,0.)
        TLorentzVector tot_lor_vec;  // initialized by (0.,0.,0.,0.)
        if (evts_from_file && cep_evt->GetnTracks()>2) {
            printf("Number of tracks exceeds 2!"); 
            gSystem->Exit(0);
        }
        // we only want 2 tracks
        TVector3 v;
        for (UInt_t kk(0); kk<2; kk++)
        {
            trk = cep_evt->GetTrack(kk);
            if (!trk) { kAllGood = kFALSE; break; }
            // momentum 
            Int_t pdg_code = 211;

            v = trk->GetMomentum(); 
            lor_vec.SetPtEtaPhiM(v.Pt(),v.Eta(),v.Phi(),TDatabasePDG::Instance()->GetParticle(pdg_code)->Mass());
            tot_lor_vec += lor_vec;
        }
        if (!kAllGood) continue;

        if (is_full_recon(cep_evt)) hInvarmass->Fill(tot_lor_vec.M());
        else hInvarmass_fd->Fill(tot_lor_vec.M());
    }
    // cursor of status display has to move to the next line
    std::cout << std::endl;
    std::cout << "100%:   conversion finished " << std::endl;
    ///////////////////////
    hist_list->Add(hInvarmass);
    hist_list->Add(hInvarmass_fd);
 
    outfile->cd();
    hist_list->Write();
    outfile->Close();
    delete outfile;

    std::cout << "\nSaving the TList in " << (output_prefix+"invar_mass_plot.root").Data() << std::endl;

}

Int_t is_full_recon(CEPEventBuffer* cepevt)
{
    if (!cepevt) {
        std::cout << "Event does not exist!" << std::endl;
        exit(EXIT_FAILURE);
    }
    // extract info if event is fully reconstruced
    // by the tracks in the TPC or not 
    TLorentzVector lv_MCparticle;       // initialized by (0., 0., 0., 0.)
    TLorentzVector lv_trks_total;       // initialized by (0., 0., 0., 0.)
    TLorentzVector lv_trk_temp;         // initialized by (0., 0., 0., 0.)
    lv_MCparticle = cepevt->GetMCParticle();
    // looping through the detected tracks
    // to see if they match the 4-mom of the
    // original particle -> if so the whole evt is
    // detected
    CEPTrackBuffer* trk = 0x0;
    for (int kk(0); kk<cepevt->GetnTracks(); kk++) {
        trk = cepevt->GetTrack(kk); 
        if (!trk) break;

        lv_trk_temp.SetVectM(trk->GetMCMomentum(), trk->GetMCMass());
        lv_trks_total += lv_trk_temp;
    }
    if ( abs_val(lv_MCparticle.M() - lv_trks_total.M()) < 10e-3 ) {
        /* std::cout << "CEP Particle: " << std::endl; */
        /* print_lv(lv_MCparticle); */
        /* std::cout << cepevt->GetnTracks() << " Particles: "; */
        /* if (cepevt->GetnTracks() > 2 ) std::cout << "           <------------- MORE THAN 2 TRACKS! "; */
        /* std::cout << "\n"; */
        /* print_lv(lv_trks_total); */
        /* std::cout << "\n--------------------------------------------------------" << std::endl; */
        return 1;
    } else return 0;
}

double abs_val(double val)
{
    if (val > 0.) return val;
    else return -val;
}


