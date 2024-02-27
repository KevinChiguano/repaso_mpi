#include <mpi.h>
#include <stdio.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <cstdlib>
#include <iostream>

std::vector<int> read_file() {
    //colocar la ruta del archivo con los datos
    std::fstream fs("./datos.txt", std::ios::in );
    std::string line;
    std::vector<int> ret;
    while( std::getline(fs, line) ){
        ret.push_back( std::stoi(line) );
    }
    fs.close();
    return ret;
}

bool busqueda(int* arr, int n, int dato){

    bool encontrado = false;

    for (int i = 0; i < n; ++i) {
        if(arr[i] == dato){
            encontrado = true;
            break;
        }
    }
    return encontrado;
}

int main(int argc, char** argv) {

    MPI_Init(&argc, &argv);

    int rank, nprocs;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    std::vector<int> arr;
    std::vector<int> arr_local;

    bool encontrado;

    std::vector<int> encontrados_totales(nprocs);

    int size, real_size, block_size, padding = 0;

    int dato = 10001;

    if(rank == 0){

        arr = read_file();
        size = arr.size();

        arr[size-1] = dato;

        if(size%nprocs != 0){
            real_size = std::ceil((double) size/nprocs) * nprocs;
            padding = real_size-size;
        }

        block_size = real_size / nprocs;

        printf("size: %d, real_size: %d, block_size: %d, padding: %d\n", size, real_size, block_size, padding);

        MPI_Send(&padding, 1, MPI_INT, nprocs-1, 0, MPI_COMM_WORLD);
    }

    MPI_Bcast(&block_size, 1, MPI_INT, 0, MPI_COMM_WORLD);
    arr_local.resize(block_size);
    printf("size: %zu, size_tot: %zu\n", arr_local.size(), arr.size());

    MPI_Scatter(arr.data(), block_size, MPI_INT,
                arr_local.data(), block_size, MPI_INT,
                0, MPI_COMM_WORLD);


    int encontrado_local = busqueda(arr_local.data(), block_size, dato) ? 1 : 0;
    printf("RANK_%d, encontrado: %s\n", rank, encontrado_local ? "true" : "false");



    MPI_Gather(&encontrado_local, 1, MPI_INT,
               encontrados_totales.data(), 1, MPI_INT,
               0, MPI_COMM_WORLD);

    // En el proceso raíz, consolidar la información
    if (rank == 0) {
        bool encontrado = false;
        for (int i = 0; i < nprocs; ++i) {
            if (encontrados_totales[i]) {
                encontrado = true;
                break;
            }
        }
        printf("El dato fue encontrado en al menos un proceso: %s\n", encontrado ? "true" : "false");
    }



    MPI_Finalize();

    return 0;
}