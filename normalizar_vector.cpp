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

double norma_vector(int* datos, int n){

    double res = 0;

    for (int i = 0; i < n; ++i) {
        res += (datos[i]*datos[i]);
    }

    return sqrt(res);

}

std::vector<double> normalizar(int* datos, int n, double norma){


    std::vector<double> res(n);
    for (int i = 0; i < n; ++i) {
        res[i] = datos[i]/norma;
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

    int size, real_size, block_size, padding = 0;

    double norma;

    std::vector<double> res;
    std::vector<double> res_local;

    if(rank == 0){
        datos = read_file();
        size = datos.size();

        res.resize(size);

        if(size%nprocs != 0){
            real_size = std::ceil((double) size/nprocs)*nprocs;
            padding = real_size - size;
            size = real_size;
        }

        block_size = size/nprocs;
        printf("size: %d, block_size: %d, padding: %d\n", size, block_size, padding);


        norma = norma_vector(datos.data(), datos.size());
        printf("norma: %.0f\n", norma);

        MPI_Send(&padding, 1, MPI_INT, nprocs-1, 0, MPI_COMM_WORLD);


    }



    MPI_Barrier(MPI_COMM_WORLD);

    MPI_Bcast(&block_size, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&norma, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    datos_local.resize(block_size);

    MPI_Scatter(datos.data(), block_size, MPI_INT,
                datos_local.data(), block_size, MPI_INT,
                0, MPI_COMM_WORLD);



    int tmp = block_size;

    if(rank == nprocs-1){
        tmp = block_size - padding;
    }

    printf("RANK_%d, norma: %.0f\n",rank, norma);

    res_local = normalizar(datos_local.data(), tmp, norma);

    MPI_Gather(res_local.data(), tmp, MPI_DOUBLE,
               res.data(), tmp, MPI_DOUBLE,
               0, MPI_COMM_WORLD);

    if(rank == 0){
        for (int i = 0; i < 10; ++i) {
            printf("%.0f, ", res[i]);

        }
        printf("\n");
    }

    MPI_Finalize();

    return 0;
}
