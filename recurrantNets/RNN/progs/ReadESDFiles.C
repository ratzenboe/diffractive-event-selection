#include <TChain.h>
#include "AliESDEvent.h"
#include <iostream>
#include <map>

void ReadESDFiles(TString esd_file)
{
    TChain* chain = new TChain("esdTree");
    chain->Add(esd_file.Data());
    AliESDEvent* esd_evt = new AliESDEvent();
    esd_evt->ReadFromTree(chain);

    for(UInt_t ii(0); ii<chain->GetEntries(); ii++){
        chain->GetEntry(ii);
        if(esd_evt->GetAliESDOld()) esd_evt->CopyFromOldESD();
        loop_func_emc(esd_evt);
        if (ii > 3) gSystem->Exit(0);
    }
}

void loop_func_emc(AliESDEvent* esd_evt)
{
    AliESDCaloCells* emcal_cells = (AliESDCaloCells*)esd_evt->GetEMCALCells();

    Int_t nCaloTracks = esd_evt->GetNumberOfCaloClusters();
    if (nCaloTracks==0) return ;

    Short_t cellNb, cellNb_max_ampl;
    Double_t cellAmpl, cellAmpl_max, cellTime;
    Float_t x[3];
    printf("\n----------------------------------\n\n");
    for (UInt_t ii(0); ii<nCaloTracks; ii++) 
    {
        AliESDCaloCluster* calo_cluster = (AliESDCaloCluster*)esd_evt->GetCaloCluster(ii);
        if (!calo_cluster->IsEMCAL()) continue;
        if (calo_cluster->GetNCells()==0) continue;
        printf("Energy of cluster: %.2f\n", calo_cluster->E());
        // print vector with 
        cellAmpl_max = 0.;
        for (UInt_t kk(0); kk<calo_cluster->GetNCells(); kk++) {
            cellNb = calo_cluster->GetCellAbsId(kk);
            cellAmpl = emcal_cells->GetCellAmplitude(cellNb);
            printf("CellNb: %i  ::  ampl: %.2f\n", cellNb, cellAmpl);
            if (cellAmpl>=cellAmpl_max) { cellAmpl_max = cellAmpl; cellNb_max_ampl = cellNb; }

            // get position
            calo_cluster->GetPosition(x);
            printf("X: %f, %f, %f\n", x[0], x[1], x[2]);
        }
        printf("Maximum ampl: %.2f, cell nb: %i\n", cellAmpl_max, cellNb_max_ampl);
        cellTime = emcal_cells->GetCellTime(cellNb_max_ampl);
        printf("time of maximum ampl cell: %E\n", cellTime);
        if (cellTime<=5.e-7) printf("!!!!!!!!!!!!!    valid cluster !!!!!!!!!!!!!!!!!!");
    }
    printf("\n----------------------------------\n\n");
}

void print_vec(TString header, std::vector<Int_t> vec)
{
    printf("%s\n", header.Data());
    for (UInt_t ii(0); ii<vec.size(); ii++) printf("%i\n",vec[ii]);
}
