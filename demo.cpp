#include <stdio.h>
#include <qiskit.h>
#include <math.h>

/* 
 * Build a 1-D transverse-field Ising Hamiltonian
*/ 
QkObs *build_hamiltonian(uint32_t num_qubits) {
    float alpha = M_PI / 8.0;

    QkBitTerm zz[2] = {QkBitTerm_Z, QkBitTerm_Z};
    QkBitTerm z[1] = {QkBitTerm_Z};
    QkBitTerm x[1] = {QkBitTerm_X};

    QkObs *obs = qk_obs_zero(num_qubits);

    for (uint32_t i = 0; i < num_qubits; i++) {
        // ZZ term
        if (i + 1 <= (num_qubits - 1)) {
            uint32_t qubits[2] = {i, i + 1};
            QkObsTerm zz_term = {{0.2, 0.}, 2, zz, qubits, num_qubits};
            qk_obs_add_term(obs, &zz_term);
        }
        
        uint32_t qubits[1] = {i};
        
        // Z term
        QkObsTerm z_term = {{-1.2 * sin(alpha), 0.}, 1, z, qubits, num_qubits};
        qk_obs_add_term(obs, &z_term);

        // X term
        QkObsTerm x_term = {{-1.2 * cos(alpha), 0.}, 1, x, qubits, num_qubits};
        qk_obs_add_term(obs, &x_term);
    }
    return obs;
}

int main() {
    // Generate the duration of the evolution
    float num_timesteps = 60;
    float evolution_time = 30.0;
    float time = evolution_time / num_timesteps;
    
    // Evolution specific parameters
    int order = 4, reps = 3;
    bool preserve_circuit_order = true;

    // Create the hamiltonian
    QkObs *obs = build_hamiltonian(6);

    // Call the evolution
    QkCircuit *qc = qk_circuit_library_suzuki_trotter(obs, order, reps, time, preserve_circuit_order, false);

    char* output = qk_circuit_draw(qc, NULL);

    printf("%s", output);

    

    qk_obs_free(obs);
    qk_circuit_free(qc);
    qk_str_free(output);
}
