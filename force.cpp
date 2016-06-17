//
// Created by Mingqiu Wang on 5/14/16.
//

#include "declaration.h"

/**
 * This function calculates the repulsion force from substrate
 * Equation: Frup = r*exp(-s/t)*(1/s/s+1/t/s) * contact_area
 *
 * @param: npheight(np.position.z)
 * @return: repulsion force (nN)
 *
 */
coord Frepulsion(const double & npheight) {

    double distance = (npheight - radius);
    const static double com_1 = 1 / proThick;
    const static double rad_2 = radius*radius;

    if (distance < bondL) {
        double contact_area = PI*(rad_2 - (npheight - bondL)*(npheight - bondL));
        double force = compressibility*((1 / distance + com_1)/distance)*
                exp(-1.0*distance*com_1);
        return coord{0, 0, contact_area*force};
    }
    else return coord{};

}

/**
 * This function calculates force from all bonds
 *
 * @param:
 *
 * @return: pair<coord, coord>, 1st is force, 2nd is torque
 *
 */
std::pair<coord, coord> Fbond(const std::set<int> & activeBond, const std::vector<bond> & bonds,
                              const std::vector<receptor> & receptors, const std::vector<ligand> & ligands) {

    int lig, rec;
    double xb, rot_f_x, rot_f_y, rot_f_z;
    coord force, sumF, sumT;

    for (int bond : activeBond) {
        lig = bonds.at(bond).ligand;
        rec = bonds.at(bond).receptor;
        xb = dist(ligands.at(lig).position, receptors.at(rec).position); // (nm)
        force =  sigma*(xb - bondL) / xb *
                (receptors.at(rec).position - ligands.at(lig).position); // (nN)
        sumF = sumF + force;
        rot_f_x = (ligands.at(lig).position_origin.y * force.z - ligands.at(lig).position_origin.z * force.y); // (nN*nm)
        rot_f_y = (ligands.at(lig).position_origin.z * force.x - ligands.at(lig).position_origin.x * force.z); // (nN*nm)
        rot_f_z = (ligands.at(lig).position_origin.x * force.y - ligands.at(lig).position_origin.y * force.x); // (nN*nm)
        // right-hand system
        sumT = sumT + coord{rot_f_x, rot_f_y, rot_f_z}; // (nN*nm)
    }
    return {sumF, sumT};
}


std::pair<coord, coord> Fshear(const double & npheight) {

    static double trans_shear_coef = 0.006*PI*viscosity*radius*shear_rate; // nN/nm
    static double trans_shear_coef2 = 0.006*0.5625*PI*viscosity*radius*radius*shear_rate; // nN/nm
    static double rot_shear_coef = 0.004 * PI*viscosity*radius*radius*radius*shear_rate; // nN*nm
    double trans_shear;
    double rot_shear, rot_shear_coef2;
    trans_shear = trans_shear_coef* npheight + trans_shear_coef2;
    rot_shear_coef2 = radius / npheight;
    rot_shear = rot_shear_coef*(1 - 0.1875*rot_shear_coef2*rot_shear_coef2*rot_shear_coef2);

    return {coord{trans_shear, 0, 0}, coord{0, rot_shear, 0}};

}

void acceleration(std::pair<coord, coord> Fbond, std::pair<coord, coord> Fshear, coord Frepulsion) {

    static double force_cal = 1 / (mass*0.001);
    static double force_cal2 = 1 / (rot_inertia*1e-12);
    np.acc = force_cal * (Fbond.first + Fshear.first + Frepulsion); // (N/kg)
    np.rot_acc = force_cal2 * (Fbond.second + Fshear.second); // (s^-2)

}