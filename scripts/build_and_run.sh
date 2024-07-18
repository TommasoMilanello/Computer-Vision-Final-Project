#!/bin/bash

# Controlla se almeno un file .cpp è stato passato come argomento
if [ "$#" -lt 1 ]; then
  echo "Utilizzo: $0 file1.cpp file2.cpp ... fileN.cpp"
  exit 1
fi

# Compila i file cpp passati come argomento
g++ "$@" -o myProgram $(pkg-config --cflags --libs opencv4)

# Controlla se la compilazione è riuscita
if [ $? -eq 0 ]; then
  echo 'Compilazione riuscita. Eseguibile creato: myProgram'
  # Esegui il programma
  ./myProgram
else
  echo 'Errore durante la compilazione.'
fi
