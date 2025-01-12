// GrObject.cpp: implementation of the CGrObject class.
//
//////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "GrObject.h"
#include "GrTexture.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#include "GrRenderer.h"
#include <GL/gl.h>

using namespace std;

//////////////////////////////////////////////////////////////////////
// CGrObject:  Generic superclass
//////////////////////////////////////////////////////////////////////

CGrObject::~CGrObject() {}

//////////////////////////////////////////////////////////////////////
// CGrPolygon:  Polygon class
//////////////////////////////////////////////////////////////////////

CGrPolygon::CGrPolygon()
{
}


void CGrPolygon::Texture(CGrTexture *p_texture)
{
    m_texture = p_texture;
}

CGrPolygon::CGrPolygon(double *a, double *b, double *c, double *d)
{
    m_vertices.push_back(CGrPoint(a[0], a[1], a[2]));
    m_vertices.push_back(CGrPoint(b[0], b[1], b[2]));
    m_vertices.push_back(CGrPoint(c[0], c[1], c[2]));
    if(d)
        m_vertices.push_back(CGrPoint(d[0], d[1], d[2]));
}

CGrPolygon::~CGrPolygon() {}

void CGrPolygon::AddVertices3(const double *a, const double *b, const double *c, bool p_computenormal)
{
    m_vertices.push_back(CGrPoint(a[0], a[1], a[2]));
    m_vertices.push_back(CGrPoint(b[0], b[1], b[2]));
    m_vertices.push_back(CGrPoint(c[0], c[1], c[2]));
    if(p_computenormal)
        ComputeNormal();
}

void CGrPolygon::AddVertices4(const double *a, const double *b, const double *c, const double *d, bool p_computenormal)
{
    m_vertices.push_back(CGrPoint(a[0], a[1], a[2]));
    m_vertices.push_back(CGrPoint(b[0], b[1], b[2]));
    m_vertices.push_back(CGrPoint(c[0], c[1], c[2]));
    m_vertices.push_back(CGrPoint(d[0], d[1], d[2]));
    if(p_computenormal)
        ComputeNormal();
}



//
// Name :         CGrPolygon::ComputeNormal()
// Description :  For flat polygons, the polygon normal can be computed
//                automatically.  That's what this function does.
// Notice :       Clears any existing normals list.
//

void CGrPolygon::ComputeNormal()
{
    m_normals.clear();

    CGrPoint normal(0, 0, 0, 0);        // Zero the normal we are computing
    std::list<CGrPoint>::iterator coord = m_vertices.begin();
    for( ; coord != m_vertices.end();  coord++)
    {
        std::list<CGrPoint>::iterator coordnext = coord; 
        coordnext++;

        if(coordnext == m_vertices.end())
            coordnext = m_vertices.begin();

        // Now, coord and coordnext are two sequential vertex 
        // pointers.
        CGrPoint &v1 = *coord;
        CGrPoint &v2 = *coordnext;

        normal[0] -= (v1[2] + v2[2]) * (v2[1] - v1[1]);
        normal[1] -= (v1[0] + v2[0]) * (v2[2] - v1[2]);
        normal[2] -= (v1[1] + v2[1]) * (v2[0] - v1[0]);
    }

    // Normalize
    double len=sqrt(normal[0]*normal[0]+normal[1]*normal[1]+normal[2]*normal[2]);
    if(len != 0.0)
    {
        normal[0]/=len;
        normal[1]/=len;
        normal[2]/=len;
    }

    // Put into the list of normals
    m_normals.push_back(normal);
}


//
// Name :         CGrPolygon::AddTexVertex3d()
// Description :  Add a vertex with an associated texture.
//

void CGrPolygon::AddTexVertex3d(double x, double y, double z, double s, double t)
{
    m_vertices.push_back(CGrPoint(x, y, z));
    m_tvertices.push_back(CGrPoint(s, t, 0));
}

void CGrPolygon::AddNormal3d(double x, double y, double z)
{
    m_normals.push_back(CGrPoint(x, y, z, 0));
    m_normals.back().Normalize3();
}

void CGrPolygon::AddNormal3dv(double *p)
{
    m_normals.push_back(CGrPoint(p[0], p[1], p[2], 0));
    m_normals.back().Normalize3();
}



//
// Name :         CGrPolygon::glRender()
// Description :  Render this polygon.  Note that this will allow
//                optional texture mapping if you want to use it
//                and it can use a single normal or a bunch.
//

void CGrPolygon::glRender()
{
    if(m_texture)
    {
        glEnable(GL_TEXTURE_2D);
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        glBindTexture(GL_TEXTURE_2D, m_texture->TexName());
    }

    list<CGrPoint>::iterator normals = m_normals.begin();
    list<CGrPoint>::iterator tvertices = m_tvertices.begin();

    glBegin(GL_POLYGON);
    for(list<CGrPoint>::iterator i=m_vertices.begin();  i!=m_vertices.end();  i++)
    {
        if(normals != m_normals.end())
        {
            normals->glNormal();
            normals++;
        }

        if(tvertices != m_tvertices.end())
        {
            tvertices->glTexVertex();
            tvertices++;
        }

        i->glVertex();
    }

    glEnd();

    if(m_texture)
    {
        glDisable(GL_TEXTURE_2D);
    }

}


void CGrPolygon::Render(CGrRenderer *p_renderer)
{
    p_renderer->RendererBeginPolygon();

    if(m_texture)
        p_renderer->RendererTexture(m_texture);

    list<CGrPoint>::iterator normals = m_normals.begin();
    list<CGrPoint>::iterator tvertices = m_tvertices.begin();

    for(list<CGrPoint>::iterator i=m_vertices.begin();  i!=m_vertices.end();  i++)
    {
        if(normals != m_normals.end())
        {
            p_renderer->RendererNormal(*normals);
            normals++;
        }

        if(tvertices != m_tvertices.end())
        {
            p_renderer->RendererTexVertex(*tvertices);
            tvertices++;
        }

        p_renderer->RendererVertex(*i);
    }

    p_renderer->RendererEndPolygon();
}


//
// Name :         CGrPolygon::RectXY()
// Description :  Helper function to create a rectangle
//                parallel to the XY plane.
//                To use, provide the lower left or upper
//                right corner, when
//                looking from the visible side and the 
//                different values in the x and y directions.   
//


void CGrPolygon::RectXY(double x, double y, double z, double dx, double dy)
{
    AddVertex3d(x, y, z);
    AddVertex3d(x + dx, y, z);
    AddVertex3d(x + dx, y + dy, z);
    AddVertex3d(x, y + dy, z);
    AddNormal3d(0, 0, (dx > 0) == (dy > 0) ? 1. : -1.);
}

//
// Name :         CGrPolygon::RectYZ()
// Description :  Helper function to create a rectangle
//                parallel to the YZ plane.  
//


void CGrPolygon::RectYZ(double x, double y, double z, double dy, double dz)
{
    AddVertex3d(x, y, z);
    AddVertex3d(x, y, z+dz);
    AddVertex3d(x, y+dy, z+dz);
    AddVertex3d(x, y+dy, z);
    AddNormal3d((dy > 0) == (dz > 0) ? -1. : 1., 0., 0.);
}


//
// Name :         CGrPolygon::RectZX()
// Description :  Helper function to create a rectangle
//                parallel to the XY plane.  
//


void CGrPolygon::RectZX(double x, double y, double z, double dz, double dx)
{
    AddVertex3d(x, y, z);
    AddVertex3d(x+dx, y, z);
    AddVertex3d(x+dx, y, z+dz);
    AddVertex3d(x, y, z+dz);
    AddNormal3d(0, (dx > 0) == (dz > 0) ? -1. : 1., 0.);
}




//////////////////////////////////////////////////////////////////////
// CGrColor:  Color class
//////////////////////////////////////////////////////////////////////

CGrColor::~CGrColor() {}



void CGrColor::glRender()
{
    glColor4dv(c);
    if(m_child)
        m_child->glRender();
}


void CGrColor::Render(CGrRenderer *p_renderer)
{
    p_renderer->RendererColor(c);
    if(m_child)
        m_child->Render(p_renderer);
}


//////////////////////////////////////////////////////////////////////
// CGrComposite:  Composite object class.
//////////////////////////////////////////////////////////////////////

CGrComposite::~CGrComposite() {}

void CGrComposite::glRender()
{
    for(list<CGrPtr<CGrObject> >::iterator i=m_children.begin();  i!=m_children.end();  i++)
        (*i)->glRender();
}


void CGrComposite::Render(CGrRenderer *p_renderer)
{
    for(list<CGrPtr<CGrObject> >::iterator i=m_children.begin();  i!=m_children.end();  i++)
        (*i)->Render(p_renderer);
}


//////////////////////////////////////////////////////////////////////
// CGrTranslate:  Translate
//////////////////////////////////////////////////////////////////////

CGrTranslate::~CGrTranslate() {}

void CGrTranslate::glRender()
{
    if(m_child)
    {
        glPushMatrix();
        glTranslated(m_x, m_y, m_z);
        m_child->glRender();
        glPopMatrix();
    }
}


void CGrTranslate::Render(CGrRenderer *p_renderer)
{
    if(m_child)
    {
        p_renderer->RendererPushMatrix();
        p_renderer->RendererTranslate(m_x, m_y, m_z);
        m_child->Render(p_renderer);
        p_renderer->RendererPopMatrix();
    }
}

//////////////////////////////////////////////////////////////////////
// CGrSgTransform  Generic transformations
//////////////////////////////////////////////////////////////////////

CGrSgTransform::~CGrSgTransform() {}

void CGrSgTransform::glRender()
{
    if(m_child)
    {
        glPushMatrix();
        CGrTransform::glMultMatrix();
        m_child->glRender();
        glPopMatrix();
    }
}


void CGrSgTransform::Render(CGrRenderer *p_renderer)
{
    if(m_child)
    {
        p_renderer->RendererPushMatrix();
        p_renderer->RendererTransform(this);
        m_child->Render(p_renderer);
        p_renderer->RendererPopMatrix();
    }
}



//////////////////////////////////////////////////////////////////////
// CGrRotate:  Rotate
//////////////////////////////////////////////////////////////////////

CGrRotate::~CGrRotate() {}

void CGrRotate::glRender()
{
    if(m_child)
    {
        glPushMatrix();
        glRotated(m_angle, m_x, m_y, m_z);
        m_child->glRender();
        glPopMatrix();
    }
}


void CGrRotate::Render(CGrRenderer *p_renderer)
{
    if(m_child)
    {
        p_renderer->RendererPushMatrix();
        p_renderer->RendererRotate(m_angle, m_x, m_y, m_z);
        m_child->Render(p_renderer);
        p_renderer->RendererPopMatrix();
    }
}


//////////////////////////////////////////////////////////////////////
// CGrMaterial:  Sets material properties.
//////////////////////////////////////////////////////////////////////

CGrMaterial::CGrMaterial()
{
    Clear();
}

CGrMaterial::CGrMaterial(float dr, float dg, float db, float da)
{
    Clear();
    m_diffuse[0] = dr;  m_diffuse[1] = dg;  m_diffuse[2] = db;  m_diffuse[3] = da;
}

CGrMaterial::CGrMaterial(float dr, float dg, float db, CGrObject *p_child)
{
    Clear();
    m_diffuse[0] = dr;  m_diffuse[1] = dg;  m_diffuse[2] = db;  m_diffuse[3] = 1.f;

    Child(p_child);
}

CGrMaterial::CGrMaterial(float dr, float dg, float db, float sr, float sg, float sb)
{
    Clear();

    m_diffuse[0] = dr; m_diffuse[1] = dg; m_diffuse[2] = db; m_diffuse[3] = 1.f;
    m_specular[0] = sr; m_specular[1] = sg; m_specular[2] = sb; m_specular[3] = 1.f;
}

CGrMaterial::CGrMaterial(float dr, float dg, float db, float sr, float sg, float sb, CGrObject *p_child)
{
    Clear();

    m_diffuse[0] = dr; m_diffuse[1] = dg; m_diffuse[2] = db; m_diffuse[3] = 1.f;
    m_specular[0] = sr; m_specular[1] = sg; m_specular[2] = sb; m_specular[3] = 1.f;
    Child(p_child);
}



CGrMaterial::~CGrMaterial() {}

void CGrMaterial::Clear()
{
	for(int c=0;  c<4;  c++)
	{
		m_diffuse[c] = 0.f;
		m_specular[c] = 0.f;
		m_specularother[c] = 0.f;
		m_ambient[c] = 0.f;
		m_emission[c] = 0.f;
	}
	m_shininess = 1.f;
}


void CGrMaterial::AmbientDiffuseSpecularShininess(const float *a, const float *d, const float *s, float sh)
{
    for(int i=0;  i<4;  i++)
    {
        m_ambient[i] = a[i];
        m_diffuse[i] = d[i];
        m_specular[i] = s[i];
    }

    m_shininess = sh;
}


void CGrMaterial::Emissive(const float *e)
{
    for(int i=0;  i<4;  i++)
        m_emission[i] = e[i];
}

void CGrMaterial::glRender()
{
    if(m_child)
    {
            glMaterialfv(GL_FRONT, GL_DIFFUSE, m_diffuse);

            glMaterialfv(GL_FRONT, GL_SPECULAR, m_specular);

            glMaterialfv(GL_FRONT, GL_AMBIENT, m_ambient);

            glMaterialfv(GL_FRONT, GL_EMISSION, m_emission);

            glMaterialfv(GL_FRONT, GL_SHININESS, &m_shininess);

        m_child->glRender();
    }
}

void CGrMaterial::glMaterial()
{
    glMaterialfv(GL_FRONT, GL_DIFFUSE, m_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, m_specular);
    glMaterialfv(GL_FRONT, GL_AMBIENT, m_ambient);
    glMaterialfv(GL_FRONT, GL_EMISSION, m_emission);
    glMaterialfv(GL_FRONT, GL_SHININESS, &m_shininess);
}


void CGrMaterial::Render(CGrRenderer *p_renderer)
{
    if(m_child)
    {
        p_renderer->RendererMaterial(this);

        m_child->Render(p_renderer);
    }
}


//
// Name :         CGrMaterial::Standard()
// Description :  There are some combinations that are so 
//                common I consider this standards.  I provide
//                an enum to set these directly.
//

void CGrMaterial::Standard(enum Standards s)
{
    int c;
    Clear();

    switch(s)
    {
    case allblack:
        for(c=0;  c<3;  c++)
        {
            m_diffuse[c] = 0.f;
            m_specular[c] = 0.f;
            m_ambient[c] = 0.f;
            m_emission[c] = 0.f;
        }

        m_diffuse[3] = 1.f;
        m_specular[3] = 1.f;
        m_ambient[3] = 1.f;
        m_emission[3] = 1.f;
        m_shininess = 0.f;
        break;

    case texture:
        // Standard for texture is white diffuse
        for(c=0;  c<3;  c++)
        {
            m_diffuse[c] = 1.f;
            m_specular[c] = 0.f;
            m_ambient[c] = 1.f;
            m_emission[c] = 0.f;
        }

        m_diffuse[3] = 1.f;
        m_specular[3] = 1.f;
        m_ambient[3] = 1.f;
        m_emission[3] = 1.f;
        m_shininess = 0.f;
        break;
    }
}



//
// Name :         CGrComposite::AddMappedRect()
// Description :  This function is a helper function that
//                adds a texture-mapped rectangle to the 
//                composite.
// Parameters :   p_texture - Pointer to texture to use
//                x1, y1 - Lower left corner (in x/y plane)
//                x2, y2 - Upper left corner
//                xd, yd - Texture division factors
//                so, to - S and T texture offsets
// Note :         An x,y point maps to s,t values like this:
//                s = x / xd + so
//                t = y / yd + to
//

void CGrComposite::AddMappedRect(CGrTexture *p_texture, double x1, double y1, 
                                 double x2, double y2, double xd, double yd,
                                 double so, double to)
{
    CGrPtr<CGrPolygon> poly = new CGrPolygon;

    poly->Texture(p_texture);
    poly->AddNormal3d(0, 0, 1);
    poly->AddTexVertex3d(x1, y1, 0, x1/xd + so, y1/yd + to);
    poly->AddTexVertex3d(x2, y1, 0, x2/xd + so, y1/yd + to);
    poly->AddTexVertex3d(x2, y2, 0, x2/xd + so, y2/yd + to);
    poly->AddTexVertex3d(x1, y2, 0, x1/xd + so, y2/yd + to);

    // Add to this composite.
    Child(poly);
}


//
// Name :         CGrComposite::Box()
// Description :  This is that always useful box creation function. Note that 
//                I created it as a member of CGrComposite so it will just
//                add the box to the composite class.
//                Box creates an upright box.  This is a box with normals on all
//                sides parallel to the coordinate axis.  
// Parameters :   x, y, z - The coordinates of tbe back, lower left corner of the box.
//                dx, dy, dz - The size of the box.
//                p_texture - An option texture to apply to the box.
// Example :      Box(5, 3, 2, 10, 5, 7)
//                This creates an untextured box with the lower, back left corner
//                at 5, 3, 2.  The box extends to x=15, y=8, and z=9.
//                This box will not have a texture applied to it.
//

void CGrComposite::Box(double x, double y, double z, double dx, double dy, double dz, CGrTexture *p_texture)
{
    CGrPtr<CGrPolygon> poly;

    // Front
    poly = new CGrPolygon;
    poly->AddNormal3d(0, 0, 1);
    poly->AddVertex3d(x, y, z + dz);
    poly->AddVertex3d(x + dx, y, z + dz);
    poly->AddVertex3d(x + dx, y + dy, z + dz);
    poly->AddVertex3d(x, y + dy, z + dz);
    if(p_texture)
    {
        poly->Texture(p_texture);
        poly->AddTex2d(0, 0);
        poly->AddTex2d(1, 0);
        poly->AddTex2d(1, 1);
        poly->AddTex2d(0, 1);
    }
    Child(poly);

    // Right
    poly = new CGrPolygon;
    poly->AddNormal3d(1, 0, 0);
    poly->AddVertex3d(x + dx, y, z + dz);
    poly->AddVertex3d(x + dx, y, z);
    poly->AddVertex3d(x + dx, y + dy, z);
    poly->AddVertex3d(x + dx, y + dy, z + dz);
    if(p_texture)
    {
        poly->Texture(p_texture);
        poly->AddTex2d(0, 0);
        poly->AddTex2d(1, 0);
        poly->AddTex2d(1, 1);
        poly->AddTex2d(0, 1);
    }
    Child(poly);

    // Back
    poly = new CGrPolygon;
    poly->AddNormal3d(0, 0, -1);
    poly->AddVertex3d(x + dx, y, z);
    poly->AddVertex3d(x, y, z);
    poly->AddVertex3d(x, y + dy, z);
    poly->AddVertex3d(x + dx, y + dy, z);
    if(p_texture)
    {
        poly->Texture(p_texture);
        poly->AddTex2d(0, 0);
        poly->AddTex2d(1, 0);
        poly->AddTex2d(1, 1);
        poly->AddTex2d(0, 1);
    }
    Child(poly);

    // Left
    poly = new CGrPolygon;
    poly->AddNormal3d(-1, 0, 0);
    poly->AddVertex3d(x, y, z);
    poly->AddVertex3d(x, y, z + dz);
    poly->AddVertex3d(x, y + dy, z + dz);
    poly->AddVertex3d(x, y + dy, z);
    if(p_texture)
    {
        poly->Texture(p_texture);
        poly->AddTex2d(0, 0);
        poly->AddTex2d(1, 0);
        poly->AddTex2d(1, 1);
        poly->AddTex2d(0, 1);
    }
    Child(poly);

    // Top
    poly = new CGrPolygon;
    poly->AddNormal3d(0, 1, 0);
    poly->AddVertex3d(x, y + dy, z + dz);
    poly->AddVertex3d(x + dx, y + dy, z + dz);
    poly->AddVertex3d(x + dx, y + dy, z);
    poly->AddVertex3d(x, y + dy, z);
    if(p_texture)
    {
        poly->Texture(p_texture);
        poly->AddTex2d(0, 0);
        poly->AddTex2d(1, 0);
        poly->AddTex2d(1, 1);
        poly->AddTex2d(0, 1);
    }
    Child(poly);

    // Bottom
    poly = new CGrPolygon;
    poly->AddNormal3d(0, -1, 0);
    poly->AddVertex3d(x, y, z);
    poly->AddVertex3d(x + dx, y, z);
    poly->AddVertex3d(x + dx, y, z + dz);
    poly->AddVertex3d(x, y, z + dz);
    if(p_texture)
    {
        poly->Texture(p_texture);
        poly->AddTex2d(0, 0);
        poly->AddTex2d(1, 0);
        poly->AddTex2d(1, 1);
        poly->AddTex2d(0, 1);
    }
    Child(poly);
}


//
// Name :         CGrComposite::SlantBox()
// Description :  I found this to be rather handy.  It's a function to create a box
//                where the far right side is lifted, but the ends remain vertical.
//

void CGrComposite::SlantBox(double x, double y, double z, double dx, double dy, double dz, double lift)
{
    CGrPtr<CGrPolygon> poly;

    // Front
    poly = new CGrPolygon;
    poly->AddNormal3d(0, 0, 1);
    poly->AddVertex3d(x, y, z + dz);
    poly->AddVertex3d(x + dx, y + lift, z + dz);
    poly->AddVertex3d(x + dx, y + lift + dy, z + dz);
    poly->AddVertex3d(x, y + dy, z + dz);
    Child(poly);

    // Right
    poly = new CGrPolygon;
    poly->AddNormal3d(1, 0, 0);
    poly->AddVertex3d(x + dx, y + lift, z + dz);
    poly->AddVertex3d(x + dx, y + lift, z);
    poly->AddVertex3d(x + dx, y + lift + dy, z);
    poly->AddVertex3d(x + dx, y + lift + dy, z + dz);
    Child(poly);

    // Back
    poly = new CGrPolygon;
    poly->AddNormal3d(0, 0, -1);
    poly->AddVertex3d(x + dx, y + lift, z);
    poly->AddVertex3d(x, y, z);
    poly->AddVertex3d(x, y + dy, z);
    poly->AddVertex3d(x + dx, y + lift + dy, z);
    Child(poly);

    // Left
    poly = new CGrPolygon;
    poly->AddNormal3d(-1, 0, 0);
    poly->AddVertex3d(x, y, z);
    poly->AddVertex3d(x, y, z + dz);
    poly->AddVertex3d(x, y + dy, z + dz);
    poly->AddVertex3d(x, y + dy, z);
    Child(poly);

    // Top
    poly = new CGrPolygon;
    poly->AddVertex3d(x, y + dy, z + dz);
    poly->AddVertex3d(x + dx, y + lift + dy, z + dz);
    poly->AddVertex3d(x + dx, y + lift + dy, z);
    poly->AddVertex3d(x, y + dy, z);
    poly->ComputeNormal();
    Child(poly);


    // Bottom
    poly = new CGrPolygon;
    poly->AddVertex3d(x, y, z);
    poly->AddVertex3d(x + dx, y + lift, z);
    poly->AddVertex3d(x + dx, y + lift, z + dz);
    poly->AddVertex3d(x, y, z + dz);
    poly->ComputeNormal();
    Child(poly);
}


void CGrComposite::Poly3(const CGrPoint &a, const CGrPoint &b, 
                         const CGrPoint &c, CGrTexture *p_texture)
{
    CGrPtr<CGrPolygon> poly = new CGrPolygon;
    poly->AddVertices3(a, b, c, true);
    Child(poly);

    if(p_texture)
    {
        poly->Texture(p_texture);
        poly->AddTex2d(0, 0);
        poly->AddTex2d(1, 0);
        poly->AddTex2d(0, 1);
    }

}

void CGrComposite::Poly4(const CGrPoint &a, const CGrPoint &b, 
                         const CGrPoint &c, const CGrPoint &d, CGrTexture *p_texture)
{
    CGrPtr<CGrPolygon> poly = new CGrPolygon;
    poly->AddVertices4(a, b, c, d, true);
    Child(poly);

    if(p_texture)
    {
        poly->Texture(p_texture);
        poly->AddTex2d(0, 0);
        poly->AddTex2d(1, 0);
        poly->AddTex2d(1, 1);
        poly->AddTex2d(0, 1);
    }

}

//
// Name :         CGrComposite::Cylinder()
// Description :  This is that always useful cylinder creation function. 
//                creates a cylinder on its side.
// Parameters :   x, y, z - The coordinates of the back, lower left corners.
//                radius, height - The size of the cylinder.
//                p_texture - An option texture to apply to the box.
// Example :      Cylinder(-10, 10, -15, 5, 15, 1000, m_texture);
//                This creates a textured cylinder with the lower, back left corner
//                at -10, 10,-152.  The box extends to radius=5, height=15
//                num_segments defines how around appearing the cylinder will be.
//                This box will have a texture applied to it.
//
void CGrComposite::Cylinder(double x, double y, double z, double radius, double height, int num_segments, CGrTexture *p_texture)
{
    CGrPtr<CGrPolygon> poly;
    double angle_top = 2 * 3.1415926 / num_segments;
    
    poly = new CGrPolygon;
    poly->AddNormal3d(0, 0, -1);

    for (int j = num_segments - 1; j >= 0; j--) {
        double angle = j * angle_top;
        double x_pos = x + radius * cos(angle);
        double y_pos = y + radius * sin(angle);
        poly->AddVertex3d(x_pos, y_pos, z);

        if (p_texture) {
            poly->Texture(p_texture);
            poly->AddTex2d(cos(angle) / 2 + 0.5, sin(angle) / 2 + 0.5);
        }
    }
    Child(poly);

    poly = new CGrPolygon;
    poly->AddNormal3d(0, 0, 1);

    for (int j = 0; j < num_segments; j++) {
        double angle = j * angle_top;
        double x_pos = x + radius * cos(angle);
        double y_pos = y + radius * sin(angle);
        poly->AddVertex3d(x_pos, y_pos, z + height);

        if (p_texture) {
            poly->Texture(p_texture);
            poly->AddTex2d(cos(angle) / 2 + 0.5, sin(angle) / 2 + 0.5);
        }
    }

    Child(poly);

    for (int i = 0; i < num_segments; i++) {
        double angle = i * angle_top;
        double x_pos1 = x + radius * cos(angle);
        double y_pos1 = y + radius * sin(angle);
        double x_pos2 = x + radius * cos(angle + angle_top);
        double y_pos2 = y + radius * sin(angle + angle_top);

        poly = new CGrPolygon;
        poly->AddNormal3d(cos(angle + angle_top / 2), sin(angle + angle_top / 2), 0);
        poly->AddVertex3d(x_pos1, y_pos1, z);
        poly->AddVertex3d(x_pos2, y_pos2, z);
        poly->AddVertex3d(x_pos2, y_pos2, z + height);
        poly->AddVertex3d(x_pos1, y_pos1, z + height);

        if (p_texture) {
            poly->Texture(p_texture);
            poly->AddTex2d(i / (double)num_segments, 0);
            poly->AddTex2d((i + 1) / (double)num_segments, 0);
            poly->AddTex2d((i + 1) / (double)num_segments, 1);
            poly->AddTex2d(i / (double)num_segments, 1);
        }

        Child(poly);
    }
}







