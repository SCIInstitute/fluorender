﻿/*    
glm.h
Nate Robins, 1997, 2000
nate@pobox.com, http://www.pobox.com/~nate

Wavefront OBJ model file format reader/writer/manipulator.

Includes routines for generating smooth normals with
preservation of edges, welding redundant vertices & texture
coordinate generation (spheremap and planar projections) + more.

*/
#if !defined(glm_h)
#define glm_h

#ifndef __glew_h__
typedef unsigned char GLubyte;
typedef float GLfloat;
typedef int GLint;
typedef unsigned int GLuint;
typedef unsigned char GLboolean;
typedef void GLvoid;
#endif

#ifndef M_PI
#define M_PI 3.14159265f
#endif

#define GLM_NONE     (0)            /* render with only vertices */
#define GLM_FLAT     (1 << 0)       /* render with facet normals */
#define GLM_SMOOTH   (1 << 1)       /* render with vertex normals */
#define GLM_TEXTURE  (1 << 2)       /* render with texture coords */
#define GLM_COLOR    (1 << 3)       /* render with colors */
#define GLM_MATERIAL (1 << 4)       /* render with materials */
#define GLM_FOG      (1 << 5)		/* render with shader */

//for texturing
typedef struct												// Create A Structure
{
	GLubyte	*imageData;										// Image Data (Up To 32 Bits)
	GLuint	bpp;											// Image Color Depth In Bits Per Pixel.
	GLuint	width;											// Image Width
	GLuint	height;											// Image Height
} TextureImage;												// Structure Name

/* GLMmaterial: Structure that defines a material in a model. 
*/
typedef struct _GLMmaterial
{
	char*   name;                 /* name of material */
	GLfloat diffuse[4];           /* diffuse component */
	GLfloat ambient[4];           /* ambient component */
	GLfloat specular[4];          /* specular component */
	GLfloat emmissive[4];         /* emmissive component */
	GLfloat shininess;            /* specular exponent */
	GLboolean havetexture;		//contains a texture
	GLuint  textureID;			//the texture id
} GLMmaterial;

/* GLMline: Structure that defines a line in a model
*/
typedef struct _GLMline
{
	GLuint numvertices;           /* number of vertices in line */
	GLuint *vindices;             /* array of line vertex indices */
} GLMline;

/* GLMtriangle: Structure that defines a triangle in a model.
*/
typedef struct _GLMtriangle
{
	GLuint vindices[3];           /* array of triangle vertex indices */
	GLuint nindices[3];           /* array of triangle normal indices */
	GLuint tindices[3];           /* array of triangle texcoord indices*/
	GLuint findex;                /* index of triangle facet normal */
} GLMtriangle;

/* GLMgroup: Structure that defines a group in a model.
*/
typedef struct _GLMgroup
{
	char*             name;           /* name of this group */
	GLuint            numtriangles;   /* number of triangles in this group */
	GLuint*           triangles;      /* array of triangle indices */
	GLuint            material;       /* index to material for group */
	struct _GLMgroup* next;           /* pointer to next group in model */
} GLMgroup;

/* GLMmodel: Structure that defines a model.
*/
typedef struct _GLMmodel
{
	char*    pathname;            /* path to this model */
	char*    mtllibname;          /* name of the material library */

	GLuint   numvertices;         /* number of vertices in model */
	GLfloat* vertices;            /* array of vertices  */

	GLuint   numnormals;          /* number of normals in model */
	GLfloat* normals;             /* array of normals */

	GLuint   numtexcoords;        /* number of texcoords in model */
	GLfloat* texcoords;           /* array of texture coordinates */

	GLuint   numfacetnorms;       /* number of facetnorms in model */
	GLfloat* facetnorms;          /* array of facetnorms */

	GLuint       numtriangles;    /* number of triangles in model */
	GLMtriangle* triangles;       /* array of triangles */

	GLuint       numlines;        /* number of lines in model */
	GLMline*     lines;           /* array of lines */

	GLuint       nummaterials;    /* number of materials in model */
	GLMmaterial* materials;       /* array of materials */

	GLuint       numgroups;       /* number of groups in model */
	GLMgroup*    groups;          /* linked list of groups */

	GLfloat position[3];          /* position of the model */

	GLboolean hastexture;

} GLMmodel;


/* glmUnitize: "unitize" a model by translating it to the origin and
* scaling it to fit in a unit cube around the origin.  Returns the
* scalefactor used.
*
* model - properly initialized GLMmodel structure 
*/
GLfloat glmUnitize(GLMmodel* model);

/* glmBoundingBox: Calculates the bounding box, which is an array of six
* floats (minx, maxx, miny, maxy, minz, maxz) of a model.
* Written by Scott D. Anderson, based on Nate Robbins's original
* glmDimensions function, then re-wrote glmDimensions to use this.
*
* model   - initialized GLMmodel structure
* boundingbox - array of 6 GLfloats (GLfloat boundingbox[6])
*/
GLvoid glmBoundingBox(GLMmodel* model, GLfloat* boundingbox);

//get the center
GLvoid glmCenter(GLMmodel* model, GLfloat* center);

/* glmDimensions: Calculates the dimensions (width, height, depth) of
* a model.
*
* model      - initialized GLMmodel structure
* dimensions - array of 3 GLfloats (GLfloat dimensions[3])
*/
GLvoid glmDimensions(GLMmodel* model, GLfloat* dimensions);

/* glmArea: Calculates the sum of face areas of a model
*
* model      - initialized GLMmodel structure
* area       - single GLfloat
*/
GLvoid glmArea(GLMmodel* model, GLfloat* scale, GLfloat* area);

/* Scale, translate and rotate the model
* Similar to OpenGL calls
*/
GLvoid glmScalef(GLMmodel* model, GLfloat x, GLfloat y, GLfloat z);

GLvoid glmTranslatef(GLMmodel* model, GLfloat x, GLfloat y, GLfloat z);

GLvoid glmRotatef(GLMmodel* model, GLfloat angle, GLfloat x, GLfloat y, GLfloat z);

/* glmScale: Scales a model by a given amount.
* 
* model - properly initialized GLMmodel structure
* scale - scalefactor (0.5 = half as large, 2.0 = twice as large)
*/
GLvoid glmScale(GLMmodel* model, GLfloat scale);

/* glmReverseWinding: Reverse the polygon winding for all polygons in
* this model.  Default winding is counter-clockwise.  Also changes
* the direction of the normals.
* 
* model - properly initialized GLMmodel structure 
*/
GLvoid glmReverseWinding(GLMmodel* model);

/* glmFacetNormals: Generates facet normals for a model (by taking the
* cross product of the two vectors derived from the sides of each
* triangle).  Assumes a counter-clockwise winding.
*
* model - initialized GLMmodel structure
*/
GLvoid glmFacetNormals(GLMmodel* model);

/* glmVertexNormals: Generates smooth vertex normals for a model.
* First builds a list of all the triangles each vertex is in.  Then
* loops through each vertex in the the list averaging all the facet
* normals of the triangles each vertex is in.  Finally, sets the
* normal index in the triangle for the vertex to the generated smooth
* normal.  If the dot product of a facet normal and the facet normal
* associated with the first triangle in the list of triangles the
* current vertex is in is greater than the cosine of the angle
* parameter to the function, that facet normal is not added into the
* average normal calculation and the corresponding vertex is given
* the facet normal.  This tends to preserve hard edges.  The angle to
* use depends on the model, but 90 degrees is usually a good start.
*
* model - initialized GLMmodel structure
* angle - maximum angle (in degrees) to smooth across
*/
GLvoid glmVertexNormals(GLMmodel* model, GLfloat angle);

/* glmLinearTexture: Generates texture coordinates according to a
* linear projection of the texture map.  It generates these by
* linearly mapping the vertices onto a square.
*
* model - pointer to initialized GLMmodel structure
*/
GLvoid glmLinearTexture(GLMmodel* model);

/* glmSpheremapTexture: Generates texture coordinates according to a
* spherical projection of the texture map.  Sometimes referred to as
* spheremap, or reflection map texture coordinates.  It generates
* these by using the normal to calculate where that vertex would map
* onto a sphere.  Since it is impossible to map something flat
* perfectly onto something spherical, there is distortion at the
* poles.  This particular implementation causes the poles along the X
* axis to be distorted.
*
* model - pointer to initialized GLMmodel structure
*/
GLvoid glmSpheremapTexture(GLMmodel* model);

/* glmDelete: Deletes a GLMmodel structure.
*
* model - initialized GLMmodel structure
*/
GLvoid glmDelete(GLMmodel* model);

/* glmClear: clear the content of the mode without freeing itself
*
* model - initialized GLMmodel structure
*/
GLvoid glmClear(GLMmodel* model);

/* glmReadOBJ: Reads a model description from a Wavefront .OBJ file.
* Returns a pointer to the created object which should be free'd with
* glmDelete().
*
* filename - name of the file containing the Wavefront .OBJ format data.  
*/
GLMmodel* glmReadOBJ(const char* filename, bool* no_fail = 0);

/* glmWriteOBJ: Writes a model description in Wavefront .OBJ format to
* a file.
*
* model    - initialized GLMmodel structure
* filename - name of the file to write the Wavefront .OBJ format data to
* mode     - a bitwise or of values describing what is written to the file
*            GLM_NONE    -  write only vertices
*            GLM_FLAT    -  write facet normals
*            GLM_SMOOTH  -  write vertex normals
*            GLM_TEXTURE -  write texture coords
*            GLM_FLAT and GLM_SMOOTH should not both be specified.
*/
GLvoid glmWriteOBJ(GLMmodel* model, const char* filename, GLuint mode);

/* glmWeld: eliminate (weld) vectors that are within an epsilon of
* each other.
*
* model      - initialized GLMmodel structure
* epsilon    - maximum difference between vertices
*              ( 0.00001 is a good start for a unitized model)
*
*/
GLvoid glmWeld(GLMmodel* model, GLfloat epsilon);

/* glmReadPPM: read a PPM raw (type P6) file.  The PPM file has a header
* that should look something like:
*
*    P6
*    # comment
*    width height max_value
*    rgbrgbrgb...
*
* where "P6" is the magic cookie which identifies the file type and
* should be the only characters on the first line followed by a
* carriage return.  Any line starting with a # mark will be treated
* as a comment and discarded.   After the magic cookie, three integer
* values are expected: width, height of the image and the maximum
* value for a pixel (max_value must be < 256 for PPM raw files).  The
* data section consists of width*height rgb triplets (one byte each)
* in binary format (i.e., such as that written with fwrite() or
* equivalent).
*
* The rgb data is returned as an array of unsigned chars (packed
* rgb).  The malloc()'d memory should be free()'d by the caller.  If
* an error occurs, an error message is sent to stderr and NULL is
* returned.
*
* filename   - name of the .ppm file.
* width      - will contain the width of the image on return.
* height     - will contain the height of the image on return.
*
*/
GLubyte* glmReadPPM(char* filename, int* width, int* height);

#endif // #ifndef glm_h
