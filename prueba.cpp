#include <mpi.h>
#include <fstream>
#include <string>
#include <vector> 
#include <cmath>

static std::vector<int> datos;

std::tuple<int, int> min_max() {
    int min = datos[0];
    int max = datos[0];

    for (int dato : datos) {
        if (dato < min)
            min = dato;
        if (dato > max)
            max = dato;
    }

    return {min, max};
}

std::vector<int> frecuencias() {
    std::vector<int> tmp(101);
    for (int dato : datos) {
        tmp[dato]++;
    }
    return tmp;
}

double promedio() {
    double suma = 0;
    for (int dato : datos) {
        suma = suma + dato;
    }
    return suma / datos.size();
}

std::vector<int> read_file() {
 std::fstream fs("./datos.txt", std::ios::in );
 std::string line;
 std::vector<int> ret;
 while( std::getline(fs, line) ){
 ret.push_back( std::stoi(line) );
 }
 fs.close();
 return ret;
} 

int main( int argc, char *argv[])
{

    datos = read_file();

    std::vector<int> freqs; 
    
    freqs = frecuencias();

    std::printf("Valor\t\t\t| Conteo\n");
    for(int i = 0; i < freqs.size(); i++){
        std::printf("%d\t\t\t| %d\n", i, freqs[i]);
    }

    int rank, nprocs;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    int rows_per_rank;
    int rows_alloc = datos.size();
    int padding;
    double promedio_total = 0;

    if(datos.size()%nprocs != 0 ){
        rows_alloc = std::ceil((double)datos.size()/nprocs)*nprocs;
        padding = rows_alloc - datos.size();
    }

    rows_per_rank = rows_alloc / nprocs;

    if(rank == 0){

        MPI_Bcast( datos.data() , datos.size() , MPI_INT , 0 , MPI_COMM_WORLD);

        std::printf("Dimension: %ld, rows_alloc: %d, rows_per_rank: %d, padding: %d\n", datos.size(), rows_alloc, rows_per_rank, padding);
        for(int rank_id = 1; rank_id<nprocs;rank_id++) {
            int start = rank_id * rows_per_rank;
            MPI_Send(&datos[start] , rows_per_rank , MPI_INT , rank_id , 0 , MPI_COMM_WORLD);
        }
        
        double promedio_parcial = promedio();

        std::printf("El promedio en el Rank_%d es: %.2f\n", rank, promedio_parcial);
        
    } else {

        std::vector<int> local(rows_per_rank);

        MPI_Bcast( datos.data() , datos.size() , MPI_INT , 0 , MPI_COMM_WORLD);
        std::printf("Rank_%d Recibiendo datos\n", rank);
        MPI_Recv(datos.data(), rows_per_rank, MPI_INT, 0,0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        std::string str = "";
        for (int i = 0; i < 10; i++){
            str = str + std::to_string(datos[i])+",";
        }
        std::printf("Datos recibidos en el Rank_%d ==> %s\n",rank, str.c_str());

        double promedio_parcial = promedio();

        std::printf("El promedio en el Rank_%d es: %.2f\n", rank, promedio_parcial);

        MPI_Reduce(&promedio_parcial, &promedio_total,1, MPI_DOUBLE,MPI_SUM, 0, MPI_COMM_WORLD);

    }

    if(rank == 0){
        std::printf("El promedio Total es: %.2f\n", (promedio_total/nprocs));
        }

    MPI_Finalize();
    
    return 0;
}