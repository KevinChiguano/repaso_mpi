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

std::vector<int> contar(int* datos, int n){

    std::vector<int> res(n, 0);

    for (int i = 0; i < n; ++i) {
        res[datos[i]]++;
    }

    return res;

}

int maximo(int* arr, int n){
    int max = arr[0];

    for (int i = 0; i < n; ++i) {
        if(arr[i] > max){
            max = arr[i];
        }
    }

    return max;
}

int minimo(int* arr, int n){
    int min = arr[0];

    for (int i = 0; i < n; ++i) {
        if(arr[i] < min){
            min = arr[i];
        }
    }

    return min;
}

int main(int argc, char** argv) {

    MPI_Init(&argc, &argv);

    int rank, nprocs;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    int block_size, size, padding = 0, real_size;

    std::vector<int> datos;
    std::vector<int> datos_local;

    std::vector<int> cout(101);
    std::vector<int> count_local;(101);

    int max, min, max_local, min_local;

    // Utilizamos printf para imprimir el mensaje
    printf("Rank %d of %d processes\n", rank, nprocs);

    if(rank == 0){

        datos = read_file();
        size = datos.size();

        cout.resize(size);

        block_size = size / nprocs;

        if(size%nprocs != 0){
            real_size = std::ceil((double) size/nprocs) * nprocs;
            padding = real_size - size;
            block_size = real_size / nprocs;
        }

        MPI_Send(&padding, 1, MPI_INT, nprocs-1, 0, MPI_COMM_WORLD);

        printf("Size: %d, Real_size: %d, block_size: %d, padding: %d\n",size, real_size, block_size, padding);


        std::map<int, int> contador;

        for (int elemento : datos) {
            contador[elemento]++;
        }

        std::cout << "Elemento\tFrecuencia\n";
        for (const auto& par : contador) {
            std::cout << par.first << "\t\t" << par.second << "\n";
        }

        int suma = 0;
        for (int elemento : datos) {
            suma += elemento;
        }

        double promedio = static_cast<double>(suma) / datos.size();
        std::cout << "\nSuma: " << suma << "\n";
        std::cout << "Promedio: " << promedio << "\n";

    }

    MPI_Bcast(&block_size, 1, MPI_INT, 0, MPI_COMM_WORLD);
    datos_local.resize(block_size);

    MPI_Scatter(datos.data(), block_size, MPI_INT,
                datos_local.data(), block_size, MPI_INT,
                0, MPI_COMM_WORLD);


    if(rank == nprocs-1){
        MPI_Recv(&padding, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        block_size = block_size - padding;
    }

    count_local = contar(datos_local.data(), block_size);

    MPI_Reduce(count_local.data(), cout.data(), 101, MPI_INT,
               MPI_SUM, 0, MPI_COMM_WORLD);

    max_local = maximo(datos_local.data(), block_size);

    MPI_Reduce(&max_local, &max, 1, MPI_INT,
               MPI_MAX, 0, MPI_COMM_WORLD);

    min_local = minimo(datos_local.data(), block_size);

    MPI_Reduce(&min_local, &min, 1, MPI_INT,
               MPI_MIN, 0, MPI_COMM_WORLD);

    if(rank == 0){

        for (int i = 0; i < 101; ++i) {
            printf("[%d] = %d\n",i, cout[i]);
        }

        printf("\n");

        printf("maximo = %d\n", max);
        printf("maximo = %d\n", min);

    }

    MPI_Finalize();

    return 0;
}