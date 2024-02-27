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

std::vector<int> frecuencia(int* datos, int n){

    std::vector<int> res(101, 0);

    for (int i = 0; i < n; ++i) {
        res[datos[i]]++;
    }

    return res;

}

int maximo(int* datos, int n){
    int max = datos[0];

    for(int i = 0; i < n; i++){
        if(datos[i] > max)
            max = datos[i];
    }

    return max;

}

int minimo(int* datos, int n){
    int min = datos[0];

    for(int i = 0; i < n; i++){
        if(datos[i] < min)
            min = datos[i];
    }
    return min;

}

int main(int argc, char** argv) {

    MPI_Init(&argc, &argv);

    int rank, nprocs;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    std::vector<int> datos;
    std::vector<int> datos_local;

    int size, real_size, block_size, padding = 0;

    std::vector<int> res(101);
    std::vector<int> res_local(101);

    if(rank == 0){

        datos = read_file();
        size = datos.size();

        if(size%nprocs != 0){
            real_size = std::ceil((double) size/nprocs)*nprocs;
            padding = real_size - size;
            size = real_size;
        }

        block_size = size / nprocs;

        printf("size: %zu, real_size: %d, block_size: %d, padding: %d\n", datos.size(), size, block_size, padding);



    }

    MPI_Bcast(&block_size, 1, MPI_INT, 0, MPI_COMM_WORLD);
    datos_local.resize(block_size);

    MPI_Scatter(datos.data(), block_size, MPI_INT,
                datos_local.data(), block_size, MPI_INT,
                0, MPI_COMM_WORLD);

    if(rank == 0){
        MPI_Send(&padding, 1, MPI_INT, nprocs-1, 0, MPI_COMM_WORLD);
    } else if(rank == nprocs-1){
        MPI_Recv(&padding, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    int tmp = block_size;

    if(rank == nprocs-1){
        tmp = block_size - padding;
    }

    res_local = frecuencia(datos_local.data(), tmp);

    MPI_Reduce(res_local.data(), res.data(), 101, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    int max, max_local, min, min_local;

    max_local = maximo(datos_local.data(), tmp);
    MPI_Reduce(&max_local, &max, 1, MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD);

    min_local = minimo(datos_local.data(), block_size);
    MPI_Reduce(&min_local, &min, 1, MPI_INT, MPI_MIN, 0, MPI_COMM_WORLD);


    if(rank == 0){

        for (int i = 0; i < 101; ++i) {
            printf("[%d], %d\n", i,res[i]);
        }

        printf("\n maximo: %d\n", max);
        printf("\n minimo: %d\n", min);

    }




    MPI_Finalize();

    return 0;
}