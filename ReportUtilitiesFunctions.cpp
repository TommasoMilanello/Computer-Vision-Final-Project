#include "ReportUtilitiesFunctions.h"
#include <fstream>
#include <iostream>
#include <iomanip>

// Implementazione della funzione per scrivere i risultati di un singolo frame in un formato temporaneo
void writeFrameResults(std::stringstream& ss, int frameNumber, const std::vector<float>& results) {
    ss << (frameNumber == 0 ? "First frame\n" : "Second frame\n");
    for (int i = 0; i < results.size(); ++i) {
        ss << " " << i << " |";
    }
    ss << "| Tot";
    ss << "\n";

    ss << std::fixed << std::setprecision(3);
    ss << results[0] << " |";
    for (int i = 1; i < results.size(); ++i) {
        ss << " " << results[i] << " |";
    }
    ss << "\n";
}

// Implementazione della funzione per formattare definitivamente la stringa e aggiungerla al file
void finalizeAndWriteToFile(const std::string& filename, const std::stringstream& ss) {
    std::ofstream outfile;
    outfile.open(filename, std::ios_base::app); // Apri il file in modalità append
    if (!outfile.is_open()) {
        std::cerr << "Errore nell'aprire il file " << filename << std::endl;
        return;
    }

    outfile << "---- New Video ----\n";
    outfile << ss.str();
    outfile.close();
}