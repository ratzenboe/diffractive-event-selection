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

void InvMassPlots(TString input_dirname, TString output_prefix="", Int_t filter=-1,
        TString path_to_evt_id_txt="", Bool_t use_bayes_proba=false)
{
    // first we check if there is a file behind the path_to_evt_id_txt
    Bool_t evts_from_file = kFALSE;
    std::vector<Int_t> evt_id_vec;
    evt_id_vec.clear();
    if (path_to_evt_id_txt.EndsWith(".txt")) {
        evts_from_file = kTRUE;
        std::ifstream txtfile(path_to_evt_id_txt.Data());
        Int_t inp_int;
    	while (txtfile >> inp_int) evt_id_vec.push_back(inp_int);
    }
    if (evt_id_vec.size() == 0 && evts_from_file) gSystem->Exit(0);
    if (evt_id_vec.size()>0) std::cout << evt_id_vec.size() << " events in txt file" << std::endl;

    if (!evts_from_file){
        if (filter==0)      output_prefix = "noV0_"+output_prefix;                  // !V0
        else if (filter==1) output_prefix = "checkSPD-hard_"+output_prefix;         // checkSPD hard
        else if (filter==2) output_prefix = "checkSPD_maxnSingle0_"+output_prefix;  // maxdnclu=maxdnfchips=maxdnfFOchips=1e5
        else if (filter==3) output_prefix = "checkSPD_maxnSingle1_"+output_prefix;  // maxnSingle=1
        else if (filter==4) output_prefix = "checkSPD_maxnSingle2_"+output_prefix;  // maxnSingle=2
        else                output_prefix = "noAD_"+output_prefix;                  // !AD
    }
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
    }
    else { printf("<E> No files found in %s", input_dirname.Data()); gSystem->Exit(1); }

    CEPRawEventBuffer* cep_raw_evt = 0x0;
    CEPEventBuffer* cep_evt = 0x0;

    cep_tree->SetBranchAddress(cep_evts_branchname,     &cep_evt);
    cep_tree->SetBranchAddress(cep_raw_evts_branchname, &cep_raw_evt);

    TFile* outfile=new TFile((output_prefix+"invar_mass_plot.root").Data(), "RECREATE");

    // event level features:
    TH1F* hInvarmass = new TH1F("invar_mass_full_recon", "", 150, 0, 2.5);
    TH1F* hInvarmass_fd = new TH1F("invar_mass_feed_down", "", 150, 0, 2.5);
    TList* hist_list = new TList();

    // LHC16-filter
    Int_t cu;
    Bool_t isDG, isNDG;
    Int_t nseltracks, n_tracks, mode;
    Int_t lhc16_filter = -999;
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
            else if (filter==1) { cu=65539; mode=0; }  // checkSPD hard
            else if (filter==2) { cu=65539; mode=1; }  // maxdnclu=maxdnfchips=maxdnfFOchips=1e5
            else if (filter==3) { cu=65539; mode=2; }  // maxnSingle=1
            else if (filter==4) { cu=65539; mode=3; }  // maxnSingle=2
            else                { cu=99; mode=0;}    // !AD
            nseltracks = LHC16Filter(cep_evt,kFALSE,cu,isDG,isNDG,mode); 
            if (isDG==kFALSE || nseltracks!=n_tracks) continue;
        }
        // initialize charge_sum with 0 for every new event
        Int_t evt_charge_sum_var = 0;
        
        // HL track info
        CEPTrackBuffer* trk = 0x0;
        TLorentzVector lor_vec;  // initialized by (0.,0.,0.,0.)
        TLorentzVector tot_lor_vec;  // initialized by (0.,0.,0.,0.)
        std::vector<Int_t> part_vec;
        part_vec.resize(cep_evt->GetnTracks());
        if (evts_from_file && cep_evt->GetnTracks()>2) {
            Bool_t kPiPlus(false), kPiMinus(false);
            for (UInt_t kk(0); kk<cep_evt->GetnTracks(); kk++){
                if (!use_bayes_proba && cep_evt->GetTrack(kk)->GetMCPID() == 211) kPiPlus=true;
                if (!use_bayes_proba && cep_evt->GetTrack(kk)->GetMCPID() == -211) kPiMinus=true;
                if (use_bayes_proba && cep_evt->GetTrack(kk)->GetPIDBayesProbability(AliPID::kPion) > 0.9 ) {
                    if (cep_evt->GetTrack(kk)->GetChargeSign() > 0) kPiPlus=true;
                    if (cep_evt->GetTrack(kk)->GetChargeSign() < 0) kPiMinus=true;
                }
            }
            if (!kPiPlus || !kPiMinus) continue;
            // create a vector with lenght cep_evt->GetnTracks()
            for (UInt_t kk(0); kk<part_vec.size(); kk++) part_vec[kk] = kk;
            while(true) {
                std::random_shuffle( part_vec.begin(), part_vec.end() );
                Int_t charge_sum = cep_evt->GetTrack(part_vec[0])->GetChargeSign() + 
                    cep_evt->GetTrack(part_vec[1])->GetChargeSign();
                Bool_t pid0pi, pid1pi;
                pid0pi = !use_bayes_proba ? (abs(cep_evt->GetTrack(part_vec[0])->GetMCPID())==211)
                   : (cep_evt->GetTrack(part_vec[0])->GetPIDBayesProbability(AliPID::kPion)=>0.9);
                pid1pi = !use_bayes_proba ? (abs(cep_evt->GetTrack(part_vec[1])->GetMCPID())==211)
                   : (cep_evt->GetTrack(part_vec[1])->GetPIDBayesProbability(AliPID::kPion)=>0.9);
                if ( charge_sum==0 && pid0pi && pid1pi ) break;
            }
        }
        // we only want 2 tracks
        TVector3 v;
        for (UInt_t kk(0); kk<2; kk++)
        {
            if (evts_from_file && cep_evt->GetnTracks()>2) track_nb = part_vec[kk];
            else track_nb = kk;
            trk = cep_evt->GetTrack(track_nb);
            if (!trk) { evt_charge_sum_var=1; break; }
            /* printf("bayes PID for pion: %f", cep_evt->GetTrack(kk)->GetPIDBayesProbability(AliPID::kPion)); */
            if (use_bayes_proba && cep_evt->GetTrack(kk)->GetPIDBayesProbability(AliPID::kPion) < 0.9 ) { evt_charge_sum_var=1; break; }
            if (!use_bayes_proba && abs(trk->GetMCPID())!=211) { evt_charge_sum_var=1; break; }
            // momentum 
            Int_t pdg_code = 211;

            v = trk->GetMomentum(); 
            lor_vec.SetPtEtaPhiM(v.Pt(),v.Eta(),v.Phi(),TDatabasePDG::Instance()->GetParticle(pdg_code)->Mass());
            tot_lor_vec += lor_vec;
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

        if (!use_bayes_proba && cep_evt->GetnTracks()==2 && is_full_recon(cep_evt)==1) hInvarmass->Fill(tot_lor_vec.M());
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


