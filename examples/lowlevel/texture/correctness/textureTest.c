#include <nfft3.h>
#include <util.h>
#include <nfsft.h>

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

/** @defgroup texture_correctness Texture: Correctness
 * This program was designed to check the correctness of the implementation
 * of the texture transforms.
 * 
 * @author Matthias Schmalz
 * @ingroup texture_examples
 * @{
 */

/** Returns @f$e^{i \cdot phi}@f$
 */ 
inline complex expi(double phi)
{
	return cos(phi) + I * sin(phi);
}

/** Returns if @f$ | x - y | \leq delta @f$.
 */
inline int equal(complex x, complex y, double delta)
{
	return cabs(x - y) <= delta;
}

/** Returns some value on a equidistant one dimensional grid.
 *
 * @par min_value - the lower margin
 * @par max_value - the upper margin
 * @par i - the index of the point that will be returned
 * Ranges from 0 to n-1.
 * @par n - the number of points on the grid
 * @par incl determines if max_value is a point on the grid.
 */
inline double equidist(double min_value, double max_value, int i, int n,
											 int incl)
{
	if (!incl) {
		n++;
	}
	return min_value
		+ ((double) (i) * (max_value - min_value)) / (double) (n - 1);
}

/** Returns @f$ \| vec1 - vec2 \|_2 @f$.
 *
 * @par length - the length of vec1 and vec2.
 */
inline double two_norm_dist(const complex * vec1, const complex * vec2,
														unsigned int length)
{
	double norm = 0;
	unsigned int i;

	for (i = 0; i < length; i++) {
		double x = cabs(vec1[i] - vec2[i]);

		norm += x * x;
	}
	norm = sqrt(norm);

	return norm;
}

/** Returns @f$ \| vec \|_2.
 *
 * @par length - the length of vec.
 */
inline double two_norm(const complex * vec, unsigned int length)
{
	double norm = 0;
	unsigned int i;

	for (i = 0; i < length; i++) {
		norm += cabs(vec[i]) * cabs(vec[i]);
	}
	norm = sqrt(norm);

	return norm;
}

/** Returns if @f$ \| vec - ref \|_2 \leq delta \cdot \|ref\|_2 @f$
 *
 * @par length - the length of vec and ref
 */
inline int equal_two_norm_rel(const complex * vec, const complex * ref,
															unsigned int length, double delta)
{
	return two_norm_dist(vec, ref, length) <= delta * two_norm(ref, length);
}

/** Initializes the nodes.
 * The nodes are taken from a
 * cartesian rectangle of h_phi_count / r_phi_count equidistant angles for the 
 * latitudes and 
 * h_theta_count / r_theta_count
 * equidistant angles for the longitudes.
 * 
 * If @f$ h\_phi\_count \cdot h\_theta\_count \cdot N2 
 * \leq r\_phi\_count \cdot r\_theta\_count @f$, i.e. if the rectangle is too
 * small, the nodes will be extended periodically.
 *
 * @par h_phi, h_theta store the pole figures.
 * @par r stores the nodes of the pole figures.
 * @par h_phi_count, h_theta_count determines the number of different 
 * @par                            latitudes / longitudes that will be taken for
 * @par                            the pole figures.
 * @par r_phi_count, r_theta_count determines the number of different
 * @par                            latitudes / longitudes that will be taken for
 * @par                            the nodes in the pole figures.
 * @par N2 - the number of nodes per pole figure
 *
 * @pre The size of h_phi and h_theta has to be 
 * @f$ h\_phi\_count \cdot h\_theta\_count @f$.
 * The size of r has to be N2.
 */ 
inline void initialize_angles(double *h_phi, double *h_theta, double *r,
															int h_phi_count, int h_theta_count, int N2,
															int r_phi_count, int r_theta_count)
{
	int k = 0;
	int o = 0;
	int s, t, j;

	for (s = 0; s < h_phi_count; s++) {
		for (t = 0; t < h_theta_count; t++) {
			int i = s * h_theta_count + t;

			h_phi[i] = equidist(0, TEXTURE_MAX_ANGLE, s, h_phi_count, 0);
			h_theta[i] = equidist(0, TEXTURE_MAX_ANGLE / 2, t, h_theta_count, 1);

			for (j = 0; j < N2; j++) {
				r[2 * (i * N2 + j)] = equidist(-TEXTURE_MAX_ANGLE / 2,
																			 TEXTURE_MAX_ANGLE / 2, k, r_phi_count,
																			 0);
				r[2 * (i * N2 + j) + 1] =
					equidist(0, TEXTURE_MAX_ANGLE / 2, o, r_theta_count, 1);

				o++;
				if (o == r_theta_count) {
					o = 0;
					k = (k + 1) % r_phi_count;
				}
			}
		}
	}
}

/** Returns @f$ Y_k^n(phi, theta)@f$.
 * See @ref sh for the definition of spherical harmonics.
 *
 * @par init determines if values from the last call should be reused.
 *
 * @pre At the first call, init has to be nonzero.
 */
complex spherical_harmonic(int k, int n, double phi, double theta, int init)
{
	static double p;
	static double p_old;
	static double old_theta;
	static int k0;
	static int n0;

	if (init || abs(n) != n0 || k < k0 || !equal(theta, old_theta, 1E-15)) {
		k0 = abs(n);
		old_theta = theta;
		p_old = 0;
		p = 1;

		for (n0 = 0; n0 < abs(n); n0++) {
			p *= sqrt((double) (2 * n0 + 1) / (double) (2 * n0 + 2))
				* sin(theta * 2 * PI / TEXTURE_MAX_ANGLE);
		}
	}

	for (; k0 < k; k0++) {
		double p_new =
			(double) (2 * k0 + 1) / sqrt((double) ((k0 - n0 + 1) * (k0 + n0 + 1)))
			* cos(theta * 2 * PI / TEXTURE_MAX_ANGLE) * p
			- sqrt((double) ((k0 - n0) * (k0 + n0)) /
						 (double) ((k0 - n0 + 1) * (k0 + n0 + 1)))
			* p_old;
		p_old = p;
		p = p_new;
	}

	return p * expi(n * phi * 2 * PI / TEXTURE_MAX_ANGLE);
}

/** A test for the inverse texture transform.
 * Runs the inverse transformation for a given number of iterations.
 * Afterwards, if the residuum is not sufficently small, prints an error 
 * message containing the relative error (see ::two_norm_dist) "diff" and the
 * absolute error goal "ref".
 *
 * The test parameters are read from a parameter file.
 * The parameters in the file are (in this order):
 * - the bandwidth N
 * - the number of longitudes of the pole figures
 * - the number of latitudes of the pole figures
 * - the number of nodes per pole figure
 * - the number of longitudes of nodes
 * - the number of latitudes of nodes
 * - the maximum number of iterations
 * - the relative error goal
 *
 * @par inp - the name of the parameter file.
 */
void simple_solver_test(const char *inp)
{
	FILE *inp_file = fopen(inp, "r");
	texture_plan plan;
	itexture_plan iplan;
	int N, N1, N2, r_phi_count, r_theta_count, h_phi_count, h_theta_count,
		max_iter;
	double delta;
	int i;
	complex *omega, *x, *omega_ref;
	double *h_phi, *h_theta, *r;
	char err_prefix[100];
	unsigned short int seed[] = { 1, 2, 3 };

	// Output user information.
	printf("*** simple_solver_test (%s)\n", inp);
	sprintf(err_prefix, "simple_solver_test failed (%s):\n", inp);

	// Read parameters.
	fscanf(inp_file, "%d%d%d%d%d%d%d%lg", &N, &h_phi_count, &h_theta_count,
				 &N2, &r_phi_count, &r_theta_count, &max_iter, &delta);
	N1 = h_phi_count * h_theta_count;
	seed48(seed);

	// Prepare input parameters.
	texture_precompute(N);
	omega = (complex *) malloc(texture_flat_length(N) * sizeof(complex));
	x = (complex *) malloc(N1 * N2 * sizeof(complex));
	omega_ref = (complex *) malloc(texture_flat_length(N) * sizeof(complex));
	h_phi = (double *) malloc(N1 * sizeof(double));
	h_theta = (double *) malloc(N2 * sizeof(double));
	r = (double *) malloc(N1 * N2 * 2 * sizeof(double));

	initialize_angles(h_phi, h_theta, r, h_phi_count, h_theta_count, N2,
										r_phi_count, r_theta_count);
	for (i = 0; i < texture_flat_length(N); i++) {
		omega_ref[i] = rand() * drand48() + I * rand() * drand48();
	}

	// Prepare plans.
	texture_init(&plan, N, N1, N2, omega_ref, x, h_phi, h_theta, r);
	texture_trafo(&plan);
	texture_set_omega(&plan, omega);

	itexture_init(&iplan, &plan);

	memset(iplan.f_hat_iter, 0,
				 texture_get_omega_length(&plan) * sizeof(complex));
	memcpy(iplan.y, x, texture_get_x_length(&plan) * sizeof(complex));

	// Run the inverse transformation.
	itexture_before_loop(&iplan);
	for (i = 0;
			 i < max_iter &&
			 !equal_two_norm_rel(iplan.f_hat_iter, omega_ref,
													 texture_get_omega_length(&plan), delta); i++) {
		itexture_loop_one_step(&iplan);
	}

	// If the residuum is not small, print an error message.
	if (!equal_two_norm_rel(iplan.f_hat_iter, omega_ref,
													texture_get_omega_length(&plan), delta)) {
		printf("%sdiff=%lg ref=%lg\n",
					 err_prefix,
					 two_norm_dist(iplan.f_hat_iter, omega_ref,
												 texture_get_omega_length(&plan)),
					 delta * two_norm(omega_ref, texture_get_omega_length(&plan)));
	}

	// Clean up.
	itexture_finalize(&iplan);
	texture_finalize(&plan);

	texture_forget();

	free(omega);
	free(x);
	free(omega_ref);
	free(h_phi);
	free(h_theta);
	free(r);

	fclose(inp_file);
}

/** A test for ::spherical_harmonic.
 * Computes the spherical harmonic with a given set of parameters and compares
 * them with given reference values.
 * In case of error there will be an error message containing degree, order,
 * longitude, latitude, reference value, computed value and absolute difference
 * (in this order).
 *
 * The parameter file contains (in this order):
 * - the number of pole figures N1
 * - the number of nodes per pole figure N2
 * - the bandwidth N
 * - the maximum absolute error
 * - N1 times:
 *   - the latitude
 *   - N2 times:
 *     - the longitude
 *     - the real part of the reference value
 *     - the imaginary part of the reference value
 *
 * @par inp - the name of the parameter file
 * 
 */
void spherical_harmonic_test(const char *inp)
{
	int N1, N2, N;
	double delta;
	int i, j, k, n0;
	char err_prefix[100];
	FILE *inp_file = fopen(inp, "r");

	// Print user information.
	sprintf(err_prefix, "spherical_harmonic_test failed (%s):\n", inp);
	printf("*** spherical_harmonic_test (%s)\n", inp);

	// Read parameters from the input file.
	fscanf(inp_file, "%d%d%d%lf", &N1, &N2, &N, &delta);

	for (i = 1; i <= N1; i++) {
		double theta;

		fscanf(inp_file, "%lE", &theta);

		for (n0 = 0; n0 <= N; n0++) {
			for (k = n0; k <= N; k++) {
				for (j = 1; j <= N2; j++) {
					int sign;
					for (sign = -1; sign <= 1; sign += 2) {
						double phi, re_out, im_out;
						complex out;
						complex res;
						int n = n0 * sign;

						fscanf(inp_file, "%lE %lE %lE", &phi, &re_out, &im_out);
						out = re_out + I * im_out;

						res = spherical_harmonic(k, n, phi, theta, 0);
						if (!equal(out, res, delta)) {
							printf("%s%d %d %g %g %g%+gi %g%+gi %g\n",
										 err_prefix, k, n, phi, theta, creal(out), cimag(out),
										 creal(res), cimag(res), cabs(out - res));
						}
					}
				}
			}
		}
	}
	fclose(inp_file);
}

//TODO Parameter
void unit_vector_test(const char *inp)
{
	FILE *inp_file = fopen(inp, "r");
	int N, h_phi_count, h_theta_count, N1, N2, r_phi_count, r_theta_count;
	double delta;
	texture_plan plan;
	complex *omega, *x;
	double *r, *h_phi, *h_theta;
	int l, m0, n0;
	char err_prefix[100];

	sprintf(err_prefix, "unit_vector_test failed (%s):\n", inp);
	printf("*** unit_vector_test (%s)\n", inp);

	fscanf(inp_file, "%d%d%d%d%d%d%lf",
				 &N, &h_phi_count, &h_theta_count, &N2, &r_phi_count, &r_theta_count,
				 &delta);
	N1 = h_phi_count * h_theta_count;

	texture_precompute(N);

	omega = (complex *) calloc(texture_flat_length(N), sizeof(complex));
	x = (complex *) malloc(N1 * N2 * sizeof(complex));
	h_phi = (double *) malloc(N1 * sizeof(double));
	h_theta = (double *) malloc(N1 * sizeof(double));
	r = (double *) malloc(N1 * N2 * 2 * sizeof(double));

	initialize_angles(h_phi, h_theta, r, h_phi_count, h_theta_count, N2,
										r_phi_count, r_theta_count);

	texture_init(&plan, N, N1, N2, omega, x, h_phi, h_theta, r);

	for (m0 = 0; m0 <= N; m0++) {
		for (n0 = 0; n0 <= N; n0++) {
			for (l = MAX(m0, n0); l <= N; l++) {
				int sign;
				int m_sign[] = { -1, 1, -1, 1 };
				int n_sign[] = { -1, -1, 1, 1 };
				for (sign = 0; sign < 4; sign++) {
					int i, j;
					int m = m_sign[sign] * m0, n = n_sign[sign] * n0;

					omega[texture_flat_index(l, m, n)] = 1;
					texture_set_omega(&plan, omega);

					texture_trafo(&plan);

					for (i = 0; i < N1; i++) {
						for (j = 0; j < N2; j++) {
							complex out =
								conj(spherical_harmonic(l, n, texture_get_h_phi(&plan)[i],
																				texture_get_h_theta(&plan)[i], 0))
								* spherical_harmonic(l, m,
																		 texture_get_r(&plan)[2 * (i * N2 + j)],
																		 texture_get_r(&plan)[2 * (i * N2 + j) +
																													1], 0);
							complex res = texture_get_x(&plan)[i * N2 + j];

							if (!equal(out, res, delta)) {
								printf
									("%sl=%d m=%d n=%d h_phi=%g h_theta=%g r_phi=%g r_theta=%g\n",
									 err_prefix, l, m, n, texture_get_h_phi(&plan)[i],
									 texture_get_h_theta(&plan)[i],
									 texture_get_r(&plan)[2 * (i * N2 + j)],
									 texture_get_r(&plan)[2 * (i * N2 + j) + 1]);
								printf("result=%g%+gi reference=%g%+gi difference=%g\n",
											 creal(res), cimag(res), creal(out), cimag(out),
											 cabs(out - res));
							}
						}
					}

					omega[texture_flat_index(l, m, n)] = 0;
					texture_set_omega(&plan, omega);
				}
			}
		}
	}

	texture_finalize(&plan);
	texture_forget();
	free(omega);
	free(x);
	free(r);
	free(h_phi);
	free(h_theta);
	fclose(inp_file);
}

void nfsft_test(const char *inp)
{
	int N, theta_count, phi_count, N1;
	double threshold, tolerance;
	FILE *inp_file = fopen(inp, "r");
	nfsft_plan_old plan;
	int l, m, i, s, t;
	complex **f_hat;
	complex *f;
	double *x;
	char err_prefix[100];

	sprintf(err_prefix, "nfsft_test failed (%s):\n", inp);
	printf("*** nfsft_test (%s)\n", inp);

	fscanf(inp_file, "%d%d%d%lg%lg",
				 &N, &phi_count, &theta_count, &threshold, &tolerance);
	N1 = phi_count * theta_count;

	nfsft_precompute_old(N, threshold, 0U);

	x = (double *) malloc(sizeof(double) * N1 * 2);
	i = 0;
	for (s = 0; s < phi_count; s++) {
		for (t = 0; t < theta_count; t++) {
			x[2 * i] = equidist(-0.5, 0.5, s, phi_count, 0);
			x[2 * i + 1] = equidist(0, 0.5, t, theta_count, 1);
			i++;
		}
	}

	f = (complex *) malloc(sizeof(complex) * N1);

	f_hat = (complex **) malloc(sizeof(complex *) * (2 * N + 1));
	for (m = 0; m <= 2 * N; m++) {
		f_hat[m] = (complex *) malloc(sizeof(complex) * (next_power_of_2(N) + 1));
	}

	for (m = 0; m <= N; m++) {
		for (l = m; l <= N; l++) {
			int mm;
			//int ll;


			for (mm = 0; mm <= 2 * N; mm++) {
				memset(f_hat[mm], 0, sizeof(complex) * (next_power_of_2(N) + 1));
			}

			f_hat[m + N][l] = 1;

			i = 0;
			for (s = 0; s < phi_count; s++) {
				for (t = 0; t < theta_count; t++) {
					double phi = equidist(-0.5, 0.5, s, phi_count, 0);
					double theta = equidist(0, 0.5, t, theta_count, 1);

					if (fabs(x[2 * i] - phi) > 1E-15
							|| fabs(x[2 * i + 1] - theta) > 1E-15) {
						printf("%sl=%d m=%d i=%d\n", err_prefix, l, m, i);
						printf("plan corrupted:\n");
						printf("expected: phi=%lf theta=%lf\n", phi, theta);
						printf("given: phi=%lf theta=%lf\n", x[2 * i], x[2 * i + 1]);
						printf("diff: phi=%lf theta=%lf\n",
									 fabs(x[2 * i] - phi), fabs(x[2 * i + 1] - theta));
					}
					i++;
				}
			}
			plan = nfsft_init_old(N, N1, f_hat, x, f, 0U);
			nfsft_trafo_old(plan);

/*			if(cabs(f_hat[m+N][l] - 1.0) > 1E-15)
			{
				printf("%sl=%d m=%d\n",
					err_prefix, l, m);
				printf("plan corrupted: expected=%lg%+lg received=%lg%+lg\n",
					1.0, 0.0, creal(f_hat[m+N][l]), cimag(f_hat[m+N][l]));
			}
					
			f_hat[m+N][l] = 0;
			for(mm = -N; mm <= N; mm++) {
				for(ll = abs(mm); ll <= N; ll++) {
					if(cabs(f_hat[mm+N][ll]) > 1E-15) {
						printf("%sl=%d m=%d\n",
							err_prefix, l, m);
						printf("plan corrupted: ll=%d mm=%d expected=0.0 given=%lg%+lg\n", 
							ll, mm, creal(f_hat[mm+N][ll]), cimag(f_hat[mm+N][ll]));
					}
				}
			}*/

			for (i = 0; i < N1; i++) {
				complex out = f[i];
				complex ref = spherical_harmonic(l, m, -x[2 * i] * TEXTURE_MAX_ANGLE,
																				 x[2 * i + 1] * TEXTURE_MAX_ANGLE, 0);
				if (!equal(ref, out, tolerance)) {
					printf("%sl=%d m=%d i=%d phi=%lg theta=%lg\n",
								 err_prefix, l, m, i, x[2 * i], x[2 * i + 1]);
					printf("out=%lg%+lgi ref=%lg%+lgi diff=%lg\n",
								 creal(out), cimag(out), creal(ref), cimag(ref),
								 cabs(ref - out));
				}
			}

//      printf("finish: %d %d\n", l, m);

			nfsft_finalize_old(plan);
		}
	}

	free(x);
	free(f);
	for (m = 0; m <= 2 * N; m++) {
		free(f_hat[m]);
	}
	free(f_hat);

	fclose(inp_file);

	nfsft_forget_old();
}

void unit_vector_adjoint_test(const char *inp)
{
	FILE *inp_file = fopen(inp, "r");
	int N, h_phi_count, h_theta_count, N1, N2, r_phi_count, r_theta_count;
	double delta;
	texture_plan plan;
	complex *omega, *x;
	double *r, *h_phi, *h_theta;
	int i, j;
	char err_prefix[100];

	sprintf(err_prefix, "unit_vector_adjoint_test failed (%s):\n", inp);
	printf("*** unit_vector_adjoint_test (%s)\n", inp);

	fscanf(inp_file, "%d%d%d%d%d%d%lf",
				 &N, &h_phi_count, &h_theta_count, &N2, &r_phi_count, &r_theta_count,
				 &delta);
	N1 = h_phi_count * h_theta_count;

	texture_precompute(N);

	omega = (complex *) malloc(texture_flat_length(N) * sizeof(complex));
	x = (complex *) calloc(N1 * N2, sizeof(complex));
	h_phi = (double *) malloc(N1 * sizeof(double));
	h_theta = (double *) malloc(N1 * sizeof(double));
	r = (double *) malloc(N1 * N2 * 2 * sizeof(double));

	initialize_angles(h_phi, h_theta, r, h_phi_count, h_theta_count, N2,
										r_phi_count, r_theta_count);

	texture_init(&plan, N, N1, N2, omega, x, h_phi, h_theta, r);

	for (i = 0; i < N1; i++) {
		for (j = 0; j < N2; j++) {
			int l, m0, n0;

			x[i * N2 + j] = 1;
			texture_set_x(&plan, x);

			texture_adjoint(&plan);

			for (m0 = 0; m0 <= N; m0++) {
				for (n0 = 0; n0 <= N; n0++) {
					for (l = MAX(m0, n0); l <= N; l++) {
						int sign;
						int m_sign[] = { 1, 1, -1, -1 };
						int n_sign[] = { 1, -1, 1, -1 };

						for (sign = 0; sign < 4; sign++) {
							int m = m0 * m_sign[sign];
							int n = n0 * n_sign[sign];

							complex out =
								spherical_harmonic(l, n, texture_get_h_phi(&plan)[i],
																	 texture_get_h_theta(&plan)[i], 0)
								*
								conj(spherical_harmonic
										 (l, m, texture_get_r(&plan)[2 * (i * N2 + j)],
											texture_get_r(&plan)[2 * (i * N2 + j) + 1], 0));
							complex res
								= texture_get_omega(&plan)[texture_flat_index(l, m, n)];

							if (!equal(out, res, delta)) {
								printf
									("%sl=%d m=%d n=%d h_phi=%g h_theta=%g r_phi=%g r_theta=%g\n",
									 err_prefix, l, m, n, texture_get_h_phi(&plan)[i],
									 texture_get_h_theta(&plan)[i],
									 texture_get_r(&plan)[2 * (i * N2 + j)],
									 texture_get_r(&plan)[2 * (i * N2 + j) + 1]);
								printf("result=%g%+gi reference=%g%+gi difference=%g\n",
											 creal(res), cimag(res), creal(out), cimag(out),
											 cabs(out - res));
							}
						}
					}
				}
			}

			x[i * N2 + j] = 0;
			texture_set_x(&plan, x);
		}
	}

	texture_finalize(&plan);
	texture_forget();
	free(omega);
	free(x);
	free(r);
	free(h_phi);
	free(h_theta);
	fclose(inp_file);
}

void linearity_test(const char *inp)
{
	FILE *inp_file = fopen(inp, "r");
	int N, h_phi_count, h_theta_count, N1, N2, r_phi_count, r_theta_count, w,
		it;
	double delta;
	char err_prefix[100];
	complex *omega, *x;
	complex *x_ref;
	double *h_phi, *h_theta, *r;
	texture_plan plan;
	int i, j, count, k;
	unsigned short int seed[] = { 1, 2, 3 };

	sprintf(err_prefix, "linearity_test failed (%s):\n", inp);
	printf("*** linearity_test (%s)\n", inp);

	fscanf(inp_file, "%d%d%d%d%d%d%d%d%lg", &N, &w, &h_phi_count,
				 &h_theta_count, &N2, &r_phi_count, &r_theta_count, &it, &delta);
	N1 = h_phi_count * h_theta_count;

	texture_precompute(N);
	seed48(seed);

	omega = (complex *) malloc(texture_flat_length(N) * sizeof(complex));
	x = (complex *) malloc(N1 * N2 * sizeof(complex));
	x_ref = (complex *) malloc(N1 * N2 * sizeof(complex));

	h_phi = (double *) malloc(N1 * sizeof(double));
	h_theta = (double *) malloc(N1 * sizeof(double));
	r = (double *) malloc(N1 * N2 * 2 * sizeof(double));
	initialize_angles(h_phi, h_theta, r, h_phi_count, h_theta_count, N2,
										r_phi_count, r_theta_count);

	texture_init(&plan, N, N1, N2, omega, x, h_phi, h_theta, r);

	for (count = 0; count < it; count++) {
		memset(omega, 0, texture_flat_length(N) * sizeof(complex));
		memset(x_ref, 0, N1 * N2 * sizeof(complex));

		for (k = 0; k < w; k++) {
			int l, m, n;
			complex offset;

			l = rand() % (N + 1);
			m = (rand() % (2 * l + 1)) - l;
			n = (rand() % (2 * l + 1)) - l;
			offset = drand48() * rand() + I * drand48() * rand();

			omega[texture_flat_index(l, m, n)] += offset;

			for (i = 0; i < N1; i++) {
				for (j = 0; j < N2; j++) {
					x_ref[i * N2 + j] += offset
						* conj(spherical_harmonic(l, n, texture_get_h_phi(&plan)[i],
																			texture_get_h_theta(&plan)[i], 0))
						* spherical_harmonic(l, m, texture_get_r(&plan)[2 * (i * N2 + j)],
																 texture_get_r(&plan)[2 * (i * N2 + j) + 1],
																 0);
				}
			}
		}
		texture_set_omega(&plan, omega);

		texture_trafo(&plan);

		if (!equal_two_norm_rel(texture_get_x(&plan), x_ref,
														texture_get_x_length(&plan), delta)) {
			printf("%scount=%d diff=%lg delta=%lg\n", err_prefix, count,
						 two_norm_dist(texture_get_x(&plan), x_ref,
													 texture_get_x_length(&plan)),
						 delta * two_norm(x_ref, texture_get_x_length(&plan)));
		}
	}

	texture_finalize(&plan);

	free(omega);
	free(x);
	free(x_ref);

	free(h_phi);
	free(h_theta);
	free(r);

	fclose(inp_file);

	texture_forget();
}

void linearity_adjoint_test(const char *inp)
{
	FILE *inp_file = fopen(inp, "r");
	int N, h_phi_count, h_theta_count, N1, N2, r_phi_count, r_theta_count, w,
		it;
	double delta;
	char err_prefix[100];
	complex *omega, *x;
	complex *omega_ref;
	double *h_phi, *h_theta, *r;
	texture_plan plan;
	int count, k;
	unsigned short int seed[] = { 1, 2, 3 };

	sprintf(err_prefix, "linearity_adjoint_test failed (%s):\n", inp);
	printf("*** linearity_adjoint_test (%s)\n", inp);

	fscanf(inp_file, "%d%d%d%d%d%d%d%d%lg", &N, &w, &h_phi_count,
				 &h_theta_count, &N2, &r_phi_count, &r_theta_count, &it, &delta);
	N1 = h_phi_count * h_theta_count;

	texture_precompute(N);
	seed48(seed);

	omega = (complex *) malloc(texture_flat_length(N) * sizeof(complex));
	x = (complex *) malloc(N1 * N2 * sizeof(complex));
	omega_ref = (complex *) malloc(texture_flat_length(N) * sizeof(complex));

	h_phi = (double *) malloc(N1 * sizeof(double));
	h_theta = (double *) malloc(N1 * sizeof(double));
	r = (double *) malloc(N1 * N2 * 2 * sizeof(double));
	initialize_angles(h_phi, h_theta, r, h_phi_count, h_theta_count, N2,
										r_phi_count, r_theta_count);

	texture_init(&plan, N, N1, N2, omega, x, h_phi, h_theta, r);

	for (count = 0; count < it; count++) {
		memset(omega_ref, 0, texture_flat_length(N) * sizeof(complex));
		memset(x, 0, N1 * N2 * sizeof(complex));

		for (k = 0; k < w; k++) {
			int i, j;
			int l, m, n;
			complex offset;

			i = rand() % N1;
			j = rand() % N2;
			offset = drand48() * rand() + I * drand48() * rand();

			x[i * N2 + j] += offset;

			for (l = 0; l <= N; l++) {
				for (m = -l; m <= l; m++) {
					for (n = -l; n <= l; n++) {
						omega_ref[texture_flat_index(l, m, n)] += offset
							* spherical_harmonic(l, n, texture_get_h_phi(&plan)[i],
																	 texture_get_h_theta(&plan)[i], 0)
							*
							conj(spherical_harmonic
									 (l, m, texture_get_r(&plan)[2 * (i * N2 + j)],
										texture_get_r(&plan)[2 * (i * N2 + j) + 1], 0));
					}
				}
			}
		}

		texture_set_x(&plan, x);

		texture_adjoint(&plan);

		if (!equal_two_norm_rel(texture_get_omega(&plan), omega_ref,
														texture_get_omega_length(&plan), delta)) {
			printf("%scount=%d diff=%lg delta=%lg\n", err_prefix, count,
						 two_norm_dist(texture_get_omega(&plan), omega_ref,
													 texture_get_omega_length(&plan)),
						 delta * two_norm(omega_ref, texture_get_omega_length(&plan)));
		}
	}

	texture_finalize(&plan);

	free(omega);
	free(x);
	free(omega_ref);

	free(h_phi);
	free(h_theta);
	free(r);

	fclose(inp_file);

	texture_forget();
}

void precompute_extreme_values_test()
{
	printf("*** precompute_extreme_values_test\n");

	texture_precompute(2);
	texture_forget();

	texture_precompute(1);
	texture_forget();

	texture_precompute(0);
	texture_forget();
}

void texture_trafo_extreme_values_test(const char *inp)
{
	FILE *inp_file = fopen(inp, "r");
	texture_plan plan;
	complex *omega;
	complex *x;
	double *h_phi, *h_theta, *r;
	int N, N1, N2;

	printf("*** texture_trafo_extreme_values_test\n");

	fscanf(inp_file, "%d%d%d", &N, &N1, &N2);

	omega = calloc(texture_flat_length(N), sizeof(complex));
	x = malloc(N1 * N2 * sizeof(complex));
	h_phi = calloc(N1, sizeof(double));
	h_theta = calloc(N1, sizeof(double));
	r = calloc(N1 * N2 * 2, sizeof(double));

	texture_precompute(N);

	texture_init(&plan, N, N1, N2, omega, x, h_phi, h_theta, r);

	texture_trafo(&plan);
	texture_adjoint(&plan);

	texture_finalize(&plan);

	texture_forget();
	free(omega);
	free(x);
	free(h_phi);
	free(h_theta);
	free(r);
	fclose(inp_file);
}

void init()
{
	spherical_harmonic(0, 0, 0, 0, 1);
}

void usage()
{
	printf("A test for the texture transform.\n");
	printf("textureTest [Options]\n");
	printf("Options:\n");
	printf("-val : use small input values for usage with valgrind\n");
}

int main(int arglen, char *argv[])
{

	if (arglen == 2 && !strcmp(argv[1], "-val")) {
		// usage with valgrind

		printf("*** Test for usage with valgrind.\n");
		printf("*** If some output not preceded by *** is produced, ");
		printf("there is some error.\n");

		init();
	
		spherical_harmonic_test("spherical_harmonic_test.inp");

		nfsft_test("nfsft_moderate_test.inp");

		//nfsft_test("nfsft_small_test.inp"); //reveals a bug in nfsft
		
		unit_vector_test("unit_vector_moderate_test.inp");

		unit_vector_adjoint_test("unit_vector_adjoint_moderate_test.inp");
		
		linearity_test("linearity_moderate_test.inp");

		linearity_adjoint_test("linearity_adjoint_moderate_test.inp");

		//precompute_extreme_values_test(); //reveals a bug in nfsft

//    texture_trafo_extreme_values_test("texture_trafo_extreme_values_test_1.inp"); //reveals a bug in nfsft
//		texture_trafo_extreme_values_test("texture_trafo_extreme_values_test_2.inp");	//reveals a bug in nfsft
//    texture_trafo_extreme_values_test("texture_trafo_extreme_values_test_3.inp");
//    texture_trafo_extreme_values_test("texture_trafo_extreme_values_test_4.inp"); //reveals a bug in nfsft

		//simple_solver_test("simple_solver_moderate_test.inp");
	} else if (arglen == 1) {
		// default usage

		printf("*** Test with default parameters.\n");
		printf("*** If some output not preceded by *** is produced, ");
		printf("there is some error.\n");

		init();
		
		spherical_harmonic_test("spherical_harmonic_test.inp");

		nfsft_test("nfsft_test.inp");

		unit_vector_test("unit_vector_test.inp");

		unit_vector_adjoint_test("unit_vector_adjoint_test.inp");
		
		linearity_test("linearity_test.inp");
		
		linearity_adjoint_test("linearity_adjoint_test.inp");

		simple_solver_test("simple_solver_test.inp");
	} else {
		usage();
	}

	return 0;
}

/**
 * @}
 */