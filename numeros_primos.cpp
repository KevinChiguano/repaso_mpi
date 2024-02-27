#include <mpi.h>
#include <stdio.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <cstdlib>
#include <iostream>

#define SIZE 100

std::vector<double> read_file() {
    //colocar la ruta del archivo con los datos
    std::fstream fs("./datos.txt", std::ios::in );
    std::string line;
    std::vector<double> ret;
    while( std::getline(fs, line) ){
        ret.push_back( std::stoi(line) );
    }
    fs.close();
    return ret;
}

std::vector<int> calcular_primos(int* datos, int n) {
    std::vector<int> res;
    for (int i = 0; i < n; ++i) {
        int num = datos[i];
        if (num != 0 && num != 1) {
            int count = 0;
            for (int j = 2; j <= sqrt(num); ++j) {
                if (num % j == 0) {
                    count++;
                    break;
                }
            }
            if (count == 0) {
                res.push_back(num);
            }
        }
    }
    return res;
}

int main(int argc, char** argv) {

    MPI_Init(&argc, &argv);

    int rank, nprocs;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    std::vector<int> datos;
    std::vector<int> datos_local;

    std::vector<int> res;
    std::vector<int> res_local;

    int tam;

    int size, real_size, block_size, padding = 0;

    if(rank == 0){

        datos.resize(SIZE);
        for (int i = 0; i < SIZE; ++i) {
            datos[i] = i;
        }

        size = datos.size();

        if(SIZE%nprocs != 0){
            real_size = std::ceil((double) SIZE/nprocs) * nprocs;
            padding = real_size - size;
            size = real_size;
        }

        block_size = size / nprocs;

        printf("size: %d, block_size: %d, padding: %d\n", size, block_size, padding);

        MPI_Send(&padding, 1, MPI_INT, nprocs-1, 0, MPI_COMM_WORLD);
    }

    MPI_Bcast(&block_size, 1, MPI_INT, 0, MPI_COMM_WORLD);
    datos_local.resize(block_size);

    MPI_Scatter(datos.data(), block_size, MPI_INT,
                datos_local.data(), block_size, MPI_INT,
                0, MPI_COMM_WORLD);

    int tmp = block_size;

    if(rank == nprocs-1){
        MPI_Recv(&padding, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        tmp = block_size - padding;
    }

    res_local = calcular_primos(datos_local.data(), tmp);

    if(rank != 0){
        int tam_local = res_local.size();
        MPI_Send(&tam_local, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
        MPI_Send(res_local.data(), tam_local, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }

    if(rank == 0){

        for (int i = 0; i < res_local.size(); ++i) {
            res.push_back(res_local[i]);
        }

        for (int i = 1; i < nprocs; ++i) {
            MPI_Recv(&tam, 1, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            res_local.resize(tam);
            MPI_Recv(res_local.data(), tam, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            for (int i = 0; i < res_local.size(); ++i) {
                res.push_back(res_local[i]);
            }

        }

        for (int i = 0; i < res.size(); ++i) {
            printf("%d, ",res[i]);
        }
        printf("\n,");


    }


    MPI_Finalize();

    return 0;
}
