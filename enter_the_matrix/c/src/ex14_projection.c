/*
** Exercise 14 (bonus) - Projection matrix
**
** The matrix that turns the 3D scene seen by the camera into the cube the GPU
** draws: s = P * V * M * p, where M places the object in the world, V moves
** the world so that the camera sits at the origin looking down -Z, and P
** (built here) squeezes the viewing frustum, the truncated pyramid between the
** near and far planes, into the unit cube. The 4th dimension is the
** homogeneous coordinate w: this matrix puts -z there, and the GPU's division
** by w is what makes distant things smaller (perspective).
**
** Conventions (the ones the display program of the subject uses):
**  - right-handed view space, the camera looks down the negative Z axis, so
**    visible points have z between -near and -far;
**  - normalised device coordinates: X and Y in [-1, 1], Z in [0, 1] (0 on the
**    near plane, 1 on the far plane), Y up;
**  - fov is the vertical field of view in RADIANS, ratio is width / height,
**    and the matrix acts on column vectors (x, y, z, 1).
**
** With f = 1 / tan(fov / 2):
**
**     [ f/ratio  0      0                 0                  ]
**     [ 0        f      0                 0                  ]
**     [ 0        0   far/(near-far)   near*far/(near-far)    ]
**     [ 0        0     -1                 0                  ]
**
** Checks (also in the tests): a point on the near plane maps to Z = 0, on the
** far plane to Z = 1, and a point on the edge of the frustum to X or Y = +-1.
**
** The display program reads the matrix column by column; column_major_text
** produces that text (one line per column). Changing near and far changes only
** the third row: it redistributes the depth precision (a very small near
** wastes almost all the Z range on the first few units), and far / near too
** large causes Z-fighting.
**
** Allowed mathematical functions: tan and the fused multiply-add. Complexity:
** O(1). Invalid parameters (fov outside (0, pi), a non-positive ratio or near,
** far <= near, NaN...) are undefined: E_INVALID is returned.
*/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "internal.h"

# define PI_F 3.14159265358979f

t_err	projection(t_mat *out, float fov, float ratio, float near_plane,
			float far_plane)
{
	float	f;
	float	d;
	float	*p;
	t_err	e;

	if (!out)
		return (E_INVALID);
	memset(out, 0, sizeof(*out));
	if (!(fov > 0.0f && fov < PI_F) || !(ratio > 0.0f && fin_f32(ratio))
		|| !(near_plane > 0.0f && fin_f32(near_plane) && fin_f32(far_plane)
			&& far_plane > near_plane))
		return (E_INVALID);
	e = mat_new(out, &g_real, 4, 4);
	if (e != E_OK)
		return (e);
	f = 1.0f / tanf(fov * 0.5f);
	d = near_plane - far_plane;
	p = out->data;
	p[0] = f / ratio;
	p[5] = f;
	p[10] = far_plane / d;
	p[11] = (near_plane * far_plane) / d;
	p[14] = -1.0f;
	return (E_OK);
}

char	*column_major_text(const t_mat *m)
{
	char	*out;
	char	cell[96];
	size_t	i;
	size_t	j;
	size_t	len;

	if (!m || !m->f)
		return (NULL);
	out = malloc(m->rows * m->cols * 100 + m->cols * 2 + 1);
	if (!out)
		return (NULL);
	len = 0;
	for (j = 0; j < m->cols; j++)
	{
		for (i = 0; i < m->rows; i++)
		{
			m->f->format(cell, sizeof(cell), CEL(m->f, m->data, i * m->cols + j));
			len += (size_t)sprintf(out + len, i ? ", %s" : "%s", cell);
		}
		out[len++] = '\n';
	}
	out[len] = '\0';
	return (out);
}
