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

std::vector<int> reemplazar(std::vector<int> datos, int n, int dato_busqueda, int dato_reemplazo){

    std::vector<int> res = datos;

    for (int i = 0; i < n; ++i) {
        if(res[i] == dato_busqueda)
            res[i] = dato_reemplazo;
    }

    return res;

}

int main(int argc, char** argv) {

    MPI_Init(&argc, &argv);

    int rank, nprocs;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    std::vector<int> datos;
    std::vector<int> dato_local;

    std::vector<int> res;
    std::vector<int> res_local;

    int size, real_size, block_size, padding = 0;

    if(rank == 0){

        datos = read_file();
        size = datos.size();

        res.resize(size);

        if(size%nprocs != 0){
            real_size = std::ceil((double) size/nprocs) * nprocs;
            padding = real_size - size;
            size = real_size;
        }

        block_size = size / nprocs;
        printf("size: %d, block_size: %d, padding: %d\n", size, block_size, padding);

        MPI_Send(&padding, 1, MPI_INT, nprocs-1, 0, MPI_COMM_WORLD);

    }

    MPI_Bcast(&block_size, 1, MPI_INT, 0, MPI_COMM_WORLD);
    dato_local.resize(block_size);

    MPI_Scatter(datos.data(), block_size, MPI_INT,
                dato_local.data(), block_size, MPI_INT,
                0, MPI_COMM_WORLD);

    int tmp = block_size;

    if(rank == nprocs-1){
        MPI_Recv(&padding, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        tmp = block_size - padding;
    }

    res_local = reemplazar(dato_local, tmp, 60, 5);

    MPI_Gather(res_local.data(), tmp, MPI_INT,
               res.data(), tmp, MPI_INT,
               0, MPI_COMM_WORLD);


    if(rank == 0){

        for (int i = 0; i < 200; ++i) {
            printf("%d, ",res[i]);
        }

        printf("\n");
        printf("size: %zu\n", res.size());

    }

    // Utilizamos printf para imprimir el mensaje
    //printf("Rank %d of %d processes\n", rank, nprocs);


    MPI_Finalize();

    return 0;
}