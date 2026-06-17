#include <stdio.h>
#include <qiskit.h>
#include <math.h>
#include <float.h>
#include <cstring>

#include "circuit/quantumcircuit.hpp"
#include "quantum_info/sparse_observable.hpp"
#include "primitives/backend_estimator_v2.hpp"
#include "service/qiskit_runtime_service_qrmi.hpp"
#include "compiler/transpiler.hpp"

using namespace Qiskit;
using namespace Qiskit::circuit;
using namespace Qiskit::providers;
using namespace Qiskit::primitives;
using namespace Qiskit::service;
using namespace Qiskit::compiler;
using namespace Qiskit::quantum_info;

using Estimator = BackendEstimatorV2;

/*
 * Util function. Build cpp SparseObservable from QkObs.
 * Defined at bottom of file.
 */
SparseObservable build_cpp_obs(QkObs *obs, size_t num_qubits);

/*
 * Build a 1-D transverse-field Ising Hamiltonian
 */
QkObs *build_hamiltonian(uint32_t num_qubits)
{
    float alpha = M_PI / 8.0;

    QkBitTerm zz[2] = {QkBitTerm_Z, QkBitTerm_Z};
    QkBitTerm z[1] = {QkBitTerm_Z};
    QkBitTerm x[1] = {QkBitTerm_X};

    QkObs *obs = qk_obs_zero(num_qubits);

    // ZZ term
    for (uint32_t i = 0; i < num_qubits; i++)
    {
        if (i + 1 <= (num_qubits - 1))
        {
            uint32_t qubits[2] = {i, i + 1};
            QkObsTerm zz_term = {{-0.2, 0.}, 2, zz, qubits, num_qubits};
            qk_obs_add_term(obs, &zz_term);
        }
    }
    // Z term
    for (uint32_t i = 0; i < num_qubits; i++)
    {
        uint32_t qubits[1] = {i};
        QkObsTerm z_term = {{-1.2 * sin(alpha), 0.}, 1, z, qubits, num_qubits};
        qk_obs_add_term(obs, &z_term);
    }
    // X term
    for (uint32_t i = 0; i < num_qubits; i++)
    {
        uint32_t qubits[1] = {i};
        QkObsTerm x_term = {{-1.2 * cos(alpha), 0.}, 1, x, qubits, num_qubits};
        qk_obs_add_term(obs, &x_term);
    }

    return obs;
}

int main()
{
    // Generate the duration of the evolution
    float num_timesteps = 10;
    float evolution_time = 2.0;
    float time = evolution_time / num_timesteps;

    // Evolution specific parameters
    int order = 4, reps = 3;
    bool preserve_circuit_order = true;

    // Create the hamiltonian
    QkObs *obs = build_hamiltonian(6);

    // Call the evolution
    QkCircuit *qc = qk_circuit_library_suzuki_trotter(obs, order, reps, time, preserve_circuit_order, false);

    // Get IBM Quantum backend
    auto service = QiskitRuntimeService();
    auto backend = service.backend("ibm_fez");

    // Configure estimator
    auto estimator = Estimator(backend);
    int shots = 100;
    double precision = sqrt(1.0 / (double)shots);

    // Transpile circuit into QASM
    QuantumCircuit transpilation_target(6, 0);
    std::vector<std::uint32_t> layout({0, 1, 2, 3, 4, 5});
    transpilation_target.set_qiskit_circuit(std::shared_ptr<rust_circuit>(qc, qk_circuit_free), layout);
    auto transpiled_circ = transpile(transpilation_target, backend);

    // Map hamiltonian to cpp bindings
    SparseObservable observable = build_cpp_obs(obs, qk_obs_num_qubits(obs));
    auto mapped_observable = observable.apply_layout(transpiled_circ.get_qubit_map());

    // Submit job
    //Estimate expectation values for t=0.0
    auto job = estimator.run({EstimatorPub(transpiled_circ, mapped_observable, precision)});
    if (job == nullptr)
    {
        return -1;
    }

    // Set estimated values
    auto result = job->result();
    auto evs = result[0].evs();
    std::vector<double> energy;
    energy.push_back(evs[0]);

    // Perform the time evolution
    auto evolved_circ = transpilation_target.copy();
    reg_t clbits;
    QkObs* evolved_obs = qk_obs_copy(obs);
    for (size_t i = 0; i < (int) num_timesteps; i++)
    {   
        // Expand the circuit to describe delta - t
        auto circ_copy = transpilation_target.copy();
        evolved_circ.compose(circ_copy, circ_copy.get_qubit_map(), clbits);
        auto transpiled_circ = transpile(evolved_circ, backend);
        // Expand the observable
        QkObs* r_obs = qk_obs_copy(obs);
        QkObs* new_obs = qk_obs_add(evolved_obs, r_obs);
        auto cppobs = build_cpp_obs(new_obs, qk_obs_num_qubits(new_obs));
        auto mapped_observable = cppobs.apply_layout(transpiled_circ.get_qubit_map());
        // Submit job
        auto job = estimator.run({EstimatorPub(transpiled_circ, mapped_observable, precision)});
        if (job == nullptr)
        {
            return -1;
        }
        auto result = job->result();
        auto evs = result[0].evs();
        energy.push_back(evs[0]);
        qk_obs_free(r_obs);
        qk_obs_free(new_obs);
    }

    // Store the result for visualization
    FILE *fptr;
    fptr = fopen("result.txt", "w");
    fprintf(fptr, "energy\n");
    for (size_t i = 0; i < energy.size(); i++)
    {
        fprintf(fptr, "%.*f\n", __DBL_DECIMAL_DIG__, energy[i]);
    }
    fclose(fptr);
    

    qk_obs_free(obs);
}

/*
 * Util function. Build cpp SparseObservable from QkObs
 */
SparseObservable build_cpp_obs(QkObs *obs, size_t num_qubits)
{
    QkComplex64 *c_coeffs = qk_obs_coeffs(obs);
    size_t *c_boundaries = qk_obs_boundaries(obs);
    size_t terms_len = qk_obs_num_terms(obs);
    std::vector<std::complex<double>> coeffs;
    std::vector<QkBitTerm> bit_terms;
    std::vector<size_t> boundaries;
    reg_t indices;
    QkObsTerm term;
    for (size_t i = 0; i < terms_len + 1; i++)
    {
        boundaries.push_back(c_boundaries[i]);
        if (i < terms_len)
        {
            qk_obs_term(obs, i, &term);
            coeffs.push_back(std::complex<double>(term.coeff.re, 0.0));
            for (size_t j = 0; j < term.len; j++)
            {
                indices.push_back(term.indices[j]);
                bit_terms.push_back(term.bit_terms[j]);
            }
        }
    }
    return SparseObservable(num_qubits, coeffs, bit_terms, indices, boundaries);
}