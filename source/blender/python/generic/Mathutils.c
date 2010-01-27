/* 
 * $Id$
 *
 * ***** BEGIN GPL LICENSE BLOCK *****
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA	02111-1307, USA.
 *
 * The Original Code is Copyright (C) 2001-2002 by NaN Holding BV.
 * All rights reserved.
 *
 * This is a new part of Blender.
 *
 * Contributor(s): Joseph Gilbert, Campbell Barton
 *
 * ***** END GPL LICENSE BLOCK *****
 */

/* Note: Changes to Mathutils since 2.4x
 * use radians rather then degrees
 * - Mathutils.MidpointVecs --> vector.lerp(other, fac)
 * - Mathutils.AngleBetweenVecs --> vector.angle(other)
 * - Mathutils.ProjectVecs --> vector.project(other)
 * - Mathutils.DifferenceQuats --> quat.difference(other)
 * - Mathutils.Slerp --> quat.slerp(other, fac)
 * - Mathutils.Rand: removed, use pythons random module
 * - Mathutils.RotationMatrix(angle, size, axis_flag, axis) --> Mathutils.RotationMatrix(angle, size, axis); merge axis & axis_flag args
 * - Matrix.scalePart --> Matrix.scale_part
 * - Matrix.translationPart --> Matrix.translation_part
 * - Matrix.rotationPart --> Matrix.rotation_part
 * - toMatrix --> to_matrix
 * - toEuler --> to_euler
 * - toQuat --> to_quat
 * - Vector.toTrackQuat --> Vector.to_track_quat
 *
 * Moved to Geometry module: Intersect, TriangleArea, TriangleNormal, QuadNormal, LineIntersect
 */

#include "Mathutils.h"

#include "BLI_math.h"
#include "PIL_time.h"
#include "BKE_utildefines.h"

//-------------------------DOC STRINGS ---------------------------
static char M_Mathutils_doc[] = "This module provides access to matrices, eulers, quaternions and vectors.";

//-----------------------------METHODS----------------------------
//-----------------quat_rotation (internal)-----------
//This function multiplies a vector/point * quat or vice versa
//to rotate the point/vector by the quaternion
//arguments should all be 3D
PyObject *quat_rotation(PyObject *arg1, PyObject *arg2)
{
	float rot[3];
	QuaternionObject *quat = NULL;
	VectorObject *vec = NULL;

	if(QuaternionObject_Check(arg1)){
		quat = (QuaternionObject*)arg1;
		if(!BaseMath_ReadCallback(quat))
			return NULL;

		if(VectorObject_Check(arg2)){
			vec = (VectorObject*)arg2;
			
			if(!BaseMath_ReadCallback(vec))
				return NULL;
			
			rot[0] = quat->quat[0]*quat->quat[0]*vec->vec[0] + 2*quat->quat[2]*quat->quat[0]*vec->vec[2] - 
				2*quat->quat[3]*quat->quat[0]*vec->vec[1] + quat->quat[1]*quat->quat[1]*vec->vec[0] + 
				2*quat->quat[2]*quat->quat[1]*vec->vec[1] + 2*quat->quat[3]*quat->quat[1]*vec->vec[2] - 
				quat->quat[3]*quat->quat[3]*vec->vec[0] - quat->quat[2]*quat->quat[2]*vec->vec[0];
			rot[1] = 2*quat->quat[1]*quat->quat[2]*vec->vec[0] + quat->quat[2]*quat->quat[2]*vec->vec[1] + 
				2*quat->quat[3]*quat->quat[2]*vec->vec[2] + 2*quat->quat[0]*quat->quat[3]*vec->vec[0] - 
				quat->quat[3]*quat->quat[3]*vec->vec[1] + quat->quat[0]*quat->quat[0]*vec->vec[1] - 
				2*quat->quat[1]*quat->quat[0]*vec->vec[2] - quat->quat[1]*quat->quat[1]*vec->vec[1];
			rot[2] = 2*quat->quat[1]*quat->quat[3]*vec->vec[0] + 2*quat->quat[2]*quat->quat[3]*vec->vec[1] + 
				quat->quat[3]*quat->quat[3]*vec->vec[2] - 2*quat->quat[0]*quat->quat[2]*vec->vec[0] - 
				quat->quat[2]*quat->quat[2]*vec->vec[2] + 2*quat->quat[0]*quat->quat[1]*vec->vec[1] - 
				quat->quat[1]*quat->quat[1]*vec->vec[2] + quat->quat[0]*quat->quat[0]*vec->vec[2];
			return newVectorObject(rot, 3, Py_NEW, NULL);
		}
	}else if(VectorObject_Check(arg1)){
		vec = (VectorObject*)arg1;
		
		if(!BaseMath_ReadCallback(vec))
			return NULL;
		
		if(QuaternionObject_Check(arg2)){
			quat = (QuaternionObject*)arg2;
			if(!BaseMath_ReadCallback(quat))
				return NULL;

			rot[0] = quat->quat[0]*quat->quat[0]*vec->vec[0] + 2*quat->quat[2]*quat->quat[0]*vec->vec[2] - 
				2*quat->quat[3]*quat->quat[0]*vec->vec[1] + quat->quat[1]*quat->quat[1]*vec->vec[0] + 
				2*quat->quat[2]*quat->quat[1]*vec->vec[1] + 2*quat->quat[3]*quat->quat[1]*vec->vec[2] - 
				quat->quat[3]*quat->quat[3]*vec->vec[0] - quat->quat[2]*quat->quat[2]*vec->vec[0];
			rot[1] = 2*quat->quat[1]*quat->quat[2]*vec->vec[0] + quat->quat[2]*quat->quat[2]*vec->vec[1] + 
				2*quat->quat[3]*quat->quat[2]*vec->vec[2] + 2*quat->quat[0]*quat->quat[3]*vec->vec[0] - 
				quat->quat[3]*quat->quat[3]*vec->vec[1] + quat->quat[0]*quat->quat[0]*vec->vec[1] - 
				2*quat->quat[1]*quat->quat[0]*vec->vec[2] - quat->quat[1]*quat->quat[1]*vec->vec[1];
			rot[2] = 2*quat->quat[1]*quat->quat[3]*vec->vec[0] + 2*quat->quat[2]*quat->quat[3]*vec->vec[1] + 
				quat->quat[3]*quat->quat[3]*vec->vec[2] - 2*quat->quat[0]*quat->quat[2]*vec->vec[0] - 
				quat->quat[2]*quat->quat[2]*vec->vec[2] + 2*quat->quat[0]*quat->quat[1]*vec->vec[1] - 
				quat->quat[1]*quat->quat[1]*vec->vec[2] + quat->quat[0]*quat->quat[0]*vec->vec[2];
			return newVectorObject(rot, 3, Py_NEW, NULL);
		}
	}

	PyErr_SetString(PyExc_RuntimeError, "quat_rotation(internal): internal problem rotating vector/point\n");
	return NULL;
	
}

//----------------------------------MATRIX FUNCTIONS--------------------
//----------------------------------Mathutils.RotationMatrix() ----------
//mat is a 1D array of floats - row[0][0],row[0][1], row[1][0], etc.
static char M_Mathutils_RotationMatrix_doc[] =
".. function:: RotationMatrix(angle, size, axis)\n"
"\n"
"   Create a matrix representing a rotation.\n"
"\n"
"   :arg angle: The angle of rotation desired.\n"
"   :type angle: float\n"
"   :arg size: The size of the rotation matrix to construct [2, 4].\n"
"   :type size: int\n"
"   :arg axis: a string in ['X', 'Y', 'Z'] or a 3D Vector Object (optional when size is 2).\n"
"   :type axis: string or vector\n"
"   :return: A new rotation matrix.\n"
"   :rtype: Matrix\n";

static PyObject *M_Mathutils_RotationMatrix(PyObject * self, PyObject * args)
{
	VectorObject *vec= NULL;
	char *axis= NULL;
	int matSize;
	float angle = 0.0f;
	float mat[16] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};

	if(!PyArg_ParseTuple(args, "fi|O", &angle, &matSize, &vec)) {
		PyErr_SetString(PyExc_TypeError, "Mathutils.RotationMatrix(angle, size, axis): expected float int and a string or vector\n");
		return NULL;
	}

	if(vec && !VectorObject_Check(vec)) {
		axis= _PyUnicode_AsString((PyObject *)vec);
		if(axis==NULL || axis[0]=='\0' || axis[1]!='\0' || axis[0] < 'X' || axis[0] > 'Z') {
			PyErr_SetString(PyExc_TypeError, "Mathutils.RotationMatrix(): 3rd argument axis value must be a 3D vector or a string in 'X', 'Y', 'Z'\n");
			return NULL;
		}
		else {
			/* use the string */
			vec= NULL;
		}
	}

#ifdef USE_MATHUTILS_DEG
	/* Clamp to -360:360 */
	while (angle<-360.0f)
		angle+=360.0;
	while (angle>360.0f)
		angle-=360.0;
#else
	while (angle<-(Py_PI*2))
		angle+=(Py_PI*2);
	while (angle>(Py_PI*2))
		angle-=(Py_PI*2);
#endif
	
	if(matSize != 2 && matSize != 3 && matSize != 4) {
		PyErr_SetString(PyExc_AttributeError, "Mathutils.RotationMatrix(): can only return a 2x2 3x3 or 4x4 matrix\n");
		return NULL;
	}
	if(matSize == 2 && (vec != NULL)) {
		PyErr_SetString(PyExc_AttributeError, "Mathutils.RotationMatrix(): cannot create a 2x2 rotation matrix around arbitrary axis\n");
		return NULL;
	}
	if((matSize == 3 || matSize == 4) && (axis == NULL) && (vec == NULL)) {
		PyErr_SetString(PyExc_AttributeError, "Mathutils.RotationMatrix(): please choose an axis of rotation for 3d and 4d matrices\n");
		return NULL;
	}
	if(vec) {
		if(vec->size != 3) {
			PyErr_SetString(PyExc_AttributeError, "Mathutils.RotationMatrix(): the vector axis must be a 3D vector\n");
			return NULL;
		}
		
		if(!BaseMath_ReadCallback(vec))
			return NULL;
		
	}
#ifdef USE_MATHUTILS_DEG
	//convert to radians
	angle = angle * (float) (Py_PI / 180);
#endif

	/* check for valid vector/axis above */
	if(vec) {
		axis_angle_to_mat3( (float (*)[3])mat,vec->vec, angle);
	}
	else if(matSize == 2) {
		//2D rotation matrix
		mat[0] = (float) cos (angle);
		mat[1] = (float) sin (angle);
		mat[2] = -((float) sin(angle));
		mat[3] = (float) cos(angle);
	} else if(strcmp(axis, "X") == 0) {
		//rotation around X
		mat[0] = 1.0f;
		mat[4] = (float) cos(angle);
		mat[5] = (float) sin(angle);
		mat[7] = -((float) sin(angle));
		mat[8] = (float) cos(angle);
	} else if(strcmp(axis, "Y") == 0) {
		//rotation around Y
		mat[0] = (float) cos(angle);
		mat[2] = -((float) sin(angle));
		mat[4] = 1.0f;
		mat[6] = (float) sin(angle);
		mat[8] = (float) cos(angle);
	} else if(strcmp(axis, "Z") == 0) {
		//rotation around Z
		mat[0] = (float) cos(angle);
		mat[1] = (float) sin(angle);
		mat[3] = -((float) sin(angle));
		mat[4] = (float) cos(angle);
		mat[8] = 1.0f;
	}
	else {
		/* should never get here */
		PyErr_SetString(PyExc_AttributeError, "Mathutils.RotationMatrix(): unknown error\n");
		return NULL;
	}

	if(matSize == 4) {
		//resize matrix
		mat[10] = mat[8];
		mat[9] = mat[7];
		mat[8] = mat[6];
		mat[7] = 0.0f;
		mat[6] = mat[5];
		mat[5] = mat[4];
		mat[4] = mat[3];
		mat[3] = 0.0f;
	}
	//pass to matrix creation
	return newMatrixObject(mat, matSize, matSize, Py_NEW, NULL);
}

static char M_Mathutils_TranslationMatrix_doc[] =
".. function:: TranslationMatrix(vector)\n"
"\n"
"   Create a matrix representing a translation.\n"
"\n"
"   :arg vector: The translation vector.\n"
"   :type vector: Vector\n"
"   :return: An identity matrix with a translation.\n"
"   :rtype: Matrix\n";

static PyObject *M_Mathutils_TranslationMatrix(PyObject * self, VectorObject * vec)
{
	float mat[16] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};
	
	if(!VectorObject_Check(vec)) {
		PyErr_SetString(PyExc_TypeError, "Mathutils.TranslationMatrix(): expected vector\n");
		return NULL;
	}
	if(vec->size != 3 && vec->size != 4) {
		PyErr_SetString(PyExc_TypeError, "Mathutils.TranslationMatrix(): vector must be 3D or 4D\n");
		return NULL;
	}
	
	if(!BaseMath_ReadCallback(vec))
		return NULL;
	
	//create a identity matrix and add translation
	unit_m4((float(*)[4]) mat);
	mat[12] = vec->vec[0];
	mat[13] = vec->vec[1];
	mat[14] = vec->vec[2];

	return newMatrixObject(mat, 4, 4, Py_NEW, NULL);
}
//----------------------------------Mathutils.ScaleMatrix() -------------
//mat is a 1D array of floats - row[0][0],row[0][1], row[1][0], etc.
static char M_Mathutils_ScaleMatrix_doc[] =
".. function:: ScaleMatrix(factor, size, axis)\n"
"\n"
"   Create a matrix representing a scaling.\n"
"\n"
"   :arg factor: The factor of scaling to apply.\n"
"   :type factor: float\n"
"   :arg size: The size of the scale matrix to construct [2, 4].\n"
"   :type size: int\n"
"   :arg axis: Direction to influence scale. (optional).\n"
"   :type axis: Vector\n"
"   :return: A new scale matrix.\n"
"   :rtype: Matrix\n";

static PyObject *M_Mathutils_ScaleMatrix(PyObject * self, PyObject * args)
{
	VectorObject *vec = NULL;
	float norm = 0.0f, factor;
	int matSize, x;
	float mat[16] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};

	if(!PyArg_ParseTuple(args, "fi|O!", &factor, &matSize, &vector_Type, &vec)) {
		PyErr_SetString(PyExc_TypeError, "Mathutils.ScaleMatrix(): expected float int and optional vector\n");
		return NULL;
	}
	if(matSize != 2 && matSize != 3 && matSize != 4) {
		PyErr_SetString(PyExc_AttributeError, "Mathutils.ScaleMatrix(): can only return a 2x2 3x3 or 4x4 matrix\n");
		return NULL;
	}
	if(vec) {
		if(vec->size > 2 && matSize == 2) {
			PyErr_SetString(PyExc_AttributeError, "Mathutils.ScaleMatrix(): please use 2D vectors when scaling in 2D\n");
			return NULL;
		}
		
		if(!BaseMath_ReadCallback(vec))
			return NULL;
		
	}
	if(vec == NULL) {	//scaling along axis
		if(matSize == 2) {
			mat[0] = factor;
			mat[3] = factor;
		} else {
			mat[0] = factor;
			mat[4] = factor;
			mat[8] = factor;
		}
	} else { //scaling in arbitrary direction
		//normalize arbitrary axis
		for(x = 0; x < vec->size; x++) {
			norm += vec->vec[x] * vec->vec[x];
		}
		norm = (float) sqrt(norm);
		for(x = 0; x < vec->size; x++) {
			vec->vec[x] /= norm;
		}
		if(matSize == 2) {
			mat[0] = 1 +((factor - 1) *(vec->vec[0] * vec->vec[0]));
			mat[1] =((factor - 1) *(vec->vec[0] * vec->vec[1]));
			mat[2] =((factor - 1) *(vec->vec[0] * vec->vec[1]));
			mat[3] = 1 + ((factor - 1) *(vec->vec[1] * vec->vec[1]));
		} else {
			mat[0] = 1 + ((factor - 1) *(vec->vec[0] * vec->vec[0]));
			mat[1] =((factor - 1) *(vec->vec[0] * vec->vec[1]));
			mat[2] =((factor - 1) *(vec->vec[0] * vec->vec[2]));
			mat[3] =((factor - 1) *(vec->vec[0] * vec->vec[1]));
			mat[4] = 1 + ((factor - 1) *(vec->vec[1] * vec->vec[1]));
			mat[5] =((factor - 1) *(vec->vec[1] * vec->vec[2]));
			mat[6] =((factor - 1) *(vec->vec[0] * vec->vec[2]));
			mat[7] =((factor - 1) *(vec->vec[1] * vec->vec[2]));
			mat[8] = 1 + ((factor - 1) *(vec->vec[2] * vec->vec[2]));
		}
	}
	if(matSize == 4) {
		//resize matrix
		mat[10] = mat[8];
		mat[9] = mat[7];
		mat[8] = mat[6];
		mat[7] = 0.0f;
		mat[6] = mat[5];
		mat[5] = mat[4];
		mat[4] = mat[3];
		mat[3] = 0.0f;
	}
	//pass to matrix creation
	return newMatrixObject(mat, matSize, matSize, Py_NEW, NULL);
}
//----------------------------------Mathutils.OrthoProjectionMatrix() ---
//mat is a 1D array of floats - row[0][0],row[0][1], row[1][0], etc.
static char M_Mathutils_OrthoProjectionMatrix_doc[] =
".. function:: OrthoProjectionMatrix(plane, size, axis)\n"
"\n"
"   Create a matrix to represent an orthographic projection.\n"
"\n"
"   :arg plane: Can be any of the following: ['X', 'Y', 'XY', 'XZ', 'YZ', 'R'], where a single axis is for a 2D matrix and 'R' requires axis is given.\n"
"   :type plane: string\n"
"   :arg size: The size of the projection matrix to construct [2, 4].\n"
"   :type size: int\n"
"   :arg axis: Arbitrary perpendicular plane vector.\n"
"   :type axis: vector (optional)\n"
"   :return: A new projection matrix.\n"
"   :rtype: Matrix\n";
static PyObject *M_Mathutils_OrthoProjectionMatrix(PyObject * self, PyObject * args)
{
	VectorObject *vec = NULL;
	char *plane;
	int matSize, x;
	float norm = 0.0f;
	float mat[16] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};
	
	if(!PyArg_ParseTuple(args, "si|O!", &plane, &matSize, &vector_Type, &vec)) {
		PyErr_SetString(PyExc_TypeError, "Mathutils.OrthoProjectionMatrix(): expected string and int and optional vector\n");
		return NULL;
	}
	if(matSize != 2 && matSize != 3 && matSize != 4) {
		PyErr_SetString(PyExc_AttributeError,"Mathutils.OrthoProjectionMatrix(): can only return a 2x2 3x3 or 4x4 matrix\n");
		return NULL;
	}
	if(vec) {
		if(vec->size > 2 && matSize == 2) {
			PyErr_SetString(PyExc_AttributeError, "Mathutils.OrthoProjectionMatrix(): please use 2D vectors when scaling in 2D\n");
			return NULL;
		}
		
		if(!BaseMath_ReadCallback(vec))
			return NULL;
		
	}
	if(vec == NULL) {	//ortho projection onto cardinal plane
		if((strcmp(plane, "X") == 0) && matSize == 2) {
			mat[0] = 1.0f;
		} else if((strcmp(plane, "Y") == 0) && matSize == 2) {
			mat[3] = 1.0f;
		} else if((strcmp(plane, "XY") == 0) && matSize > 2) {
			mat[0] = 1.0f;
			mat[4] = 1.0f;
		} else if((strcmp(plane, "XZ") == 0) && matSize > 2) {
			mat[0] = 1.0f;
			mat[8] = 1.0f;
		} else if((strcmp(plane, "YZ") == 0) && matSize > 2) {
			mat[4] = 1.0f;
			mat[8] = 1.0f;
		} else {
			PyErr_SetString(PyExc_AttributeError, "Mathutils.OrthoProjectionMatrix(): unknown plane - expected: X, Y, XY, XZ, YZ\n");
			return NULL;
		}
	} else { //arbitrary plane
		//normalize arbitrary axis
		for(x = 0; x < vec->size; x++) {
			norm += vec->vec[x] * vec->vec[x];
		}
		norm = (float) sqrt(norm);
		for(x = 0; x < vec->size; x++) {
			vec->vec[x] /= norm;
		}
		if((strcmp(plane, "R") == 0) && matSize == 2) {
			mat[0] = 1 - (vec->vec[0] * vec->vec[0]);
			mat[1] = -(vec->vec[0] * vec->vec[1]);
			mat[2] = -(vec->vec[0] * vec->vec[1]);
			mat[3] = 1 - (vec->vec[1] * vec->vec[1]);
		} else if((strcmp(plane, "R") == 0) && matSize > 2) {
			mat[0] = 1 - (vec->vec[0] * vec->vec[0]);
			mat[1] = -(vec->vec[0] * vec->vec[1]);
			mat[2] = -(vec->vec[0] * vec->vec[2]);
			mat[3] = -(vec->vec[0] * vec->vec[1]);
			mat[4] = 1 - (vec->vec[1] * vec->vec[1]);
			mat[5] = -(vec->vec[1] * vec->vec[2]);
			mat[6] = -(vec->vec[0] * vec->vec[2]);
			mat[7] = -(vec->vec[1] * vec->vec[2]);
			mat[8] = 1 - (vec->vec[2] * vec->vec[2]);
		} else {
			PyErr_SetString(PyExc_AttributeError, "Mathutils.OrthoProjectionMatrix(): unknown plane - expected: 'r' expected for axis designation\n");
			return NULL;
		}
	}
	if(matSize == 4) {
		//resize matrix
		mat[10] = mat[8];
		mat[9] = mat[7];
		mat[8] = mat[6];
		mat[7] = 0.0f;
		mat[6] = mat[5];
		mat[5] = mat[4];
		mat[4] = mat[3];
		mat[3] = 0.0f;
	}
	//pass to matrix creation
	return newMatrixObject(mat, matSize, matSize, Py_NEW, NULL);
}

static char M_Mathutils_ShearMatrix_doc[] =
".. function:: ShearMatrix(plane, factor, size)\n"
"\n"
"   Create a matrix to represent an shear transformation.\n"
"\n"
"   :arg plane: Can be any of the following: ['X', 'Y', 'XY', 'XZ', 'YZ'], where a single axis is for a 2D matrix.\n"
"   :type plane: string\n"
"   :arg factor: The factor of shear to apply.\n"
"   :type factor: float\n"
"   :arg size: The size of the shear matrix to construct [2, 4].\n"
"   :type size: int\n"
"   :return: A new shear matrix.\n"
"   :rtype: Matrix\n";

static PyObject *M_Mathutils_ShearMatrix(PyObject * self, PyObject * args)
{
	int matSize;
	char *plane;
	float factor;
	float mat[16] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};

	if(!PyArg_ParseTuple(args, "sfi", &plane, &factor, &matSize)) {
		PyErr_SetString(PyExc_TypeError,"Mathutils.ShearMatrix(): expected string float and int\n");
		return NULL;
	}
	if(matSize != 2 && matSize != 3 && matSize != 4) {
		PyErr_SetString(PyExc_AttributeError,"Mathutils.ShearMatrix(): can only return a 2x2 3x3 or 4x4 matrix\n");
		return NULL;
	}

	if((strcmp(plane, "X") == 0)
	    && matSize == 2) {
		mat[0] = 1.0f;
		mat[2] = factor;
		mat[3] = 1.0f;
	} else if((strcmp(plane, "Y") == 0) && matSize == 2) {
		mat[0] = 1.0f;
		mat[1] = factor;
		mat[3] = 1.0f;
	} else if((strcmp(plane, "XY") == 0) && matSize > 2) {
		mat[0] = 1.0f;
		mat[4] = 1.0f;
		mat[6] = factor;
		mat[7] = factor;
	} else if((strcmp(plane, "XZ") == 0) && matSize > 2) {
		mat[0] = 1.0f;
		mat[3] = factor;
		mat[4] = 1.0f;
		mat[5] = factor;
		mat[8] = 1.0f;
	} else if((strcmp(plane, "YZ") == 0) && matSize > 2) {
		mat[0] = 1.0f;
		mat[1] = factor;
		mat[2] = factor;
		mat[4] = 1.0f;
		mat[8] = 1.0f;
	} else {
		PyErr_SetString(PyExc_AttributeError, "Mathutils.ShearMatrix(): expected: x, y, xy, xz, yz or wrong matrix size for shearing plane\n");
		return NULL;
	}
	if(matSize == 4) {
		//resize matrix
		mat[10] = mat[8];
		mat[9] = mat[7];
		mat[8] = mat[6];
		mat[7] = 0.0f;
		mat[6] = mat[5];
		mat[5] = mat[4];
		mat[4] = mat[3];
		mat[3] = 0.0f;
	}
	//pass to matrix creation
	return newMatrixObject(mat, matSize, matSize, Py_NEW, NULL);
}

/* Utility functions */

/*---------------------- EXPP_FloatsAreEqual -------------------------
  Floating point comparisons 
  floatStep = number of representable floats allowable in between
   float A and float B to be considered equal. */
int EXPP_FloatsAreEqual(float A, float B, int floatSteps)
{
	int a, b, delta;
    assert(floatSteps > 0 && floatSteps < (4 * 1024 * 1024));
    a = *(int*)&A;
    if (a < 0)	
		a = 0x80000000 - a;
    b = *(int*)&B;
    if (b < 0)	
		b = 0x80000000 - b;
    delta = abs(a - b);
    if (delta <= floatSteps)	
		return 1;
    return 0;
}
/*---------------------- EXPP_VectorsAreEqual -------------------------
  Builds on EXPP_FloatsAreEqual to test vectors */
int EXPP_VectorsAreEqual(float *vecA, float *vecB, int size, int floatSteps)
{
	int x;
	for (x=0; x< size; x++){
		if (EXPP_FloatsAreEqual(vecA[x], vecB[x], floatSteps) == 0)
			return 0;
	}
	return 1;
}


/* Mathutils Callbacks */

/* for mathutils internal use only, eventually should re-alloc but to start with we only have a few users */
Mathutils_Callback *mathutils_callbacks[8] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};

int Mathutils_RegisterCallback(Mathutils_Callback *cb)
{
	int i;
	
	/* find the first free slot */
	for(i= 0; mathutils_callbacks[i]; i++) {
		if(mathutils_callbacks[i]==cb) /* alredy registered? */
			return i;
	}
	
	mathutils_callbacks[i] = cb;
	return i;
}

/* use macros to check for NULL */
int _BaseMathObject_ReadCallback(BaseMathObject *self)
{
	Mathutils_Callback *cb= mathutils_callbacks[self->cb_type];
	if(cb->get(self->cb_user, self->cb_subtype, self->data))
		return 1;

	PyErr_Format(PyExc_SystemError, "%s user has become invalid", Py_TYPE(self)->tp_name);
	return 0;
}

int _BaseMathObject_WriteCallback(BaseMathObject *self)
{
	Mathutils_Callback *cb= mathutils_callbacks[self->cb_type];
	if(cb->set(self->cb_user, self->cb_subtype, self->data))
		return 1;

	PyErr_Format(PyExc_SystemError, "%s user has become invalid", Py_TYPE(self)->tp_name);
	return 0;
}

int _BaseMathObject_ReadIndexCallback(BaseMathObject *self, int index)
{
	Mathutils_Callback *cb= mathutils_callbacks[self->cb_type];
	if(cb->get_index(self->cb_user, self->cb_subtype, self->data, index))
		return 1;

	PyErr_Format(PyExc_SystemError, "%s user has become invalid", Py_TYPE(self)->tp_name);
	return 0;
}

int _BaseMathObject_WriteIndexCallback(BaseMathObject *self, int index)
{
	Mathutils_Callback *cb= mathutils_callbacks[self->cb_type];
	if(cb->set_index(self->cb_user, self->cb_subtype, self->data, index))
		return 1;

	PyErr_Format(PyExc_SystemError, "%s user has become invalid", Py_TYPE(self)->tp_name);
	return 0;
}

/* BaseMathObject generic functions for all mathutils types */
PyObject *BaseMathObject_getOwner( BaseMathObject * self, void *type )
{
	PyObject *ret= self->cb_user ? self->cb_user : Py_None;
	Py_INCREF(ret);
	return ret;
}

PyObject *BaseMathObject_getWrapped( BaseMathObject *self, void *type )
{
	return PyBool_FromLong((self->wrapped == Py_WRAP) ? 1:0);
}

void BaseMathObject_dealloc(BaseMathObject * self)
{
	/* only free non wrapped */
	if(self->wrapped != Py_WRAP)
		PyMem_Free(self->data);

	Py_XDECREF(self->cb_user);
	Py_TYPE(self)->tp_free(self); // PyObject_DEL(self); // breaks subtypes
}

/*----------------------------MODULE INIT-------------------------*/
struct PyMethodDef M_Mathutils_methods[] = {
	{"RotationMatrix", (PyCFunction) M_Mathutils_RotationMatrix, METH_VARARGS, M_Mathutils_RotationMatrix_doc},
	{"ScaleMatrix", (PyCFunction) M_Mathutils_ScaleMatrix, METH_VARARGS, M_Mathutils_ScaleMatrix_doc},
	{"ShearMatrix", (PyCFunction) M_Mathutils_ShearMatrix, METH_VARARGS, M_Mathutils_ShearMatrix_doc},
	{"TranslationMatrix", (PyCFunction) M_Mathutils_TranslationMatrix, METH_O, M_Mathutils_TranslationMatrix_doc},
	{"OrthoProjectionMatrix", (PyCFunction) M_Mathutils_OrthoProjectionMatrix,  METH_VARARGS, M_Mathutils_OrthoProjectionMatrix_doc},
	{NULL, NULL, 0, NULL}
};

static struct PyModuleDef M_Mathutils_module_def = {
	PyModuleDef_HEAD_INIT,
	"Mathutils",  /* m_name */
	M_Mathutils_doc,  /* m_doc */
	0,  /* m_size */
	M_Mathutils_methods,  /* m_methods */
	0,  /* m_reload */
	0,  /* m_traverse */
	0,  /* m_clear */
	0,  /* m_free */
};

PyObject *Mathutils_Init(void)
{
	PyObject *submodule;
	
	if( PyType_Ready( &vector_Type ) < 0 )
		return NULL;
	if( PyType_Ready( &matrix_Type ) < 0 )
		return NULL;	
	if( PyType_Ready( &euler_Type ) < 0 )
		return NULL;
	if( PyType_Ready( &quaternion_Type ) < 0 )
		return NULL;
	
	submodule = PyModule_Create(&M_Mathutils_module_def);
	PyDict_SetItemString(PySys_GetObject("modules"), M_Mathutils_module_def.m_name, submodule);
	
	/* each type has its own new() function */
	PyModule_AddObject( submodule, "Vector",		(PyObject *)&vector_Type );
	PyModule_AddObject( submodule, "Matrix",		(PyObject *)&matrix_Type );
	PyModule_AddObject( submodule, "Euler",			(PyObject *)&euler_Type );
	PyModule_AddObject( submodule, "Quaternion",	(PyObject *)&quaternion_Type );
	
	mathutils_matrix_vector_cb_index= Mathutils_RegisterCallback(&mathutils_matrix_vector_cb);

	return (submodule);
}
