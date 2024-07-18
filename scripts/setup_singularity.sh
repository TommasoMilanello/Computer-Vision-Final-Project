#!/bin/bash

# Avvia il container Singularity e resta in esso
apptainer shell -B /run /opt/singularity/opencv.sif
