// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __DATA_CUBE_H__
#define  __DATA_CUBE_H__

////////////////////////////////////////////////////////////////////////

#include <vector>

////////////////////////////////////////////////////////////////////////

class DataCube {

    public:

        // Constructors
        DataCube();

        DataCube(const DataCube&);

        // Destructor
        ~DataCube();

        void clear();

        void erase();

        // Set methods
        void set_size(int nx, int ny, int nz);

        void set(double value, int i, int j, int k);

        void set_constant(double value);

        // Get sizes
        int nx() const;
        int ny() const;
        int nz() const;

        bool shape_equal(const DataCube&);

        void check_shape_equal(const DataCube&);

        // Index get
        double get(int i, int j, int k) const;

        // Assignment
        void assign(const DataCube&);

        // Arithmetic methods
        void increment(void);

        void add_assign(const DataCube&);

        void subtract_assign(const DataCube&);

        void multiply_assign(const DataCube&);

        void divide_assign(int);

        void divide_assign(const DataCube&);

        void min_assign(const DataCube&);

        void max_assign(const DataCube&);

        void square();

        void square_root();

        // Index operator
        double operator()(int i, int j, int k) const;

        // Assignment operator
        DataCube& operator=(const DataCube&);

        // Arithmetic operators
        DataCube& operator+=(const DataCube&);

        DataCube& operator-=(const DataCube&);

        DataCube& operator*=(const DataCube&);

        DataCube& operator/=(const DataCube&);

        double* data();

    private:

        int Nx;
        int Ny;
        int Nz;
        int Nxyz; // Nx * Ny * Nz

        std::vector<double> Data;

        void init_from_scratch();
};

#endif  /*  __DATA_CUBE_H__  */


////////////////////////////////////////////////////////////////////////

inline int DataCube::nx() const { return Nx; }
inline int DataCube::ny() const { return Ny; }
inline int DataCube::nz() const { return Nz; }

inline double* DataCube::data() { return Data.data(); }

////////////////////////////////////////////////////////////////////////
