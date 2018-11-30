/****************************************************************************

 Copyright (C) 2002-2014 Gilles Debunne. All rights reserved.

 This file is part of the BasicViewer library version 2.7.1.

 http://www.libqglviewer.com - contact@libqglviewer.com

 This file may be used under the terms of the GNU General Public License
 versions 2.0 or 3.0 as published by the Free Software Foundation and
 appearing in the LICENSE file included in the packaging of this file.
 In addition, as a special exception, Gilles Debunne gives you certain
 additional rights, described in the file GPL_EXCEPTION in this package.

 libQGLViewer uses dual licensing. Commercial/proprietary software must
 purchase a libQGLViewer Commercial License.

 This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.

*****************************************************************************/

#ifndef QGLVIEWER_FRAME_H
#define QGLVIEWER_FRAME_H

#include <model/math_types.h>
#include "constraint.h"


#include <set>



class FrameObserver {
public:
	virtual void onFrameModified() = 0;
};


/*! \brief The Frame class represents a coordinate system, defined by a position
  and an orientation. \class Frame frame.h BasicViewer/frame.h

  A Frame is a 3D coordinate system, represented by a position() and an
  orientation(). The order of these transformations is important: the Frame is
  first translated \e and \e then rotated around the new translated origin.

  A Frame is useful to define the position and orientation of a 3D rigid object,
  using its matrix() method, as shown below: \code
  // Builds a Frame at position (0.5,0,0) and oriented such that its Y axis is
  along the (1,1,1)
  // direction. One could also have used setPosition() and setOrientation().
  Frame fr(vec3(0.5,0,0), quat(vec3(0,1,0), vec3(1,1,1)));
  glPushMatrix();
  glMultMatrixd(fr.matrix());
  // Draw your object here, in the local fr coordinate system.
  glPopMatrix();
  \endcode

  Many functions are provided to transform a 3D point from one coordinate system
  (Frame) to an other: see coordinatesOf(), inverseCoordinatesOf(),
  coordinatesOfIn(), coordinatesOfFrom()...

  You may also want to transform a 3D vector (such as a normal), which
  corresponds to applying only the rotational part of the frame transformation:
  see transformOf() and inverseTransformOf(). See the <a
  href="../examples/frameTransform.html">frameTransform example</a> for an
  illustration.

  The translation() and the rotation() that are encapsulated in a Frame can also
  be used to represent a \e rigid \e transformation of space. Such a
  transformation can also be interpreted as a change of coordinate system, and
  the coordinate system conversion functions actually allow you to use a Frame
  as a rigid transformation. Use inverseCoordinatesOf() (resp. coordinatesOf())
  to apply the transformation (resp. its inverse). Note the inversion.

  <h3>Hierarchy of Frames</h3>

  The position and the orientation of a Frame are actually defined with respect
  to a referenceFrame(). The default referenceFrame() is the world coordinate
  system (represented by a \c NULL referenceFrame()). If you setReferenceFrame()
  to a different Frame, you must then differentiate:

  \arg the \e local translation() and rotation(), defined with respect to the
  referenceFrame(),

  \arg the \e global position() and orientation(), always defined with respect
  to the world coordinate system.

  A Frame is actually defined by its translation() with respect to its
  referenceFrame(), and then by a rotation() of the coordinate system around the
  new translated origin.

  This terminology for \e local (translation() and rotation()) and \e global
  (position() and orientation()) definitions is used in all the methods' names
  and should be sufficient to prevent ambiguities. These notions are obviously
  identical when the referenceFrame() is \c NULL, i.e. when the Frame is defined
  in the world coordinate system (the one you are in at the beginning of the
  BasicViewer::draw() method, see the <a href="../introduction.html">introduction
  page</a>).

  Frames can hence easily be organized in a tree hierarchy, which root is the
  world coordinate system. A loop in the hierarchy would result in an
  inconsistent (multiple) Frame definition.
  settingAsReferenceFrameWillCreateALoop() checks this and prevents
  setReferenceFrame() from creating such a loop.

  This frame hierarchy is used in methods like coordinatesOfIn(),
  coordinatesOfFrom()... which allow coordinates (or vector) conversions from a
  Frame to any other one (including the world coordinate system).

  However, one must note that this hierarchical representation is internal to
  the Frame classes. When the Frames represent OpenGL coordinates system, one
  should map this hierarchical representation to the OpenGL GL_MODELVIEW matrix
  stack. See the matrix() documentation for details.

  <h3>Constraints</h3>

  An interesting feature of Frames is that their displacements can be
  constrained. When a Constraint is attached to a Frame, it filters the input of
  translate() and rotate(), and only the resulting filtered motion is applied to
  the Frame. The default constraint() is \c NULL resulting in no filtering. Use
  setConstraint() to attach a Constraint to a frame.

  Constraints are especially usefull for the ManipulatedFrame instances, in
  order to forbid some mouse motions. See the <a
  href="../examples/constrainedFrame.html">constrainedFrame</a>, <a
  href="../examples/constrainedCamera.html">constrainedCamera</a> and <a
  href="../examples/luxo.html">luxo</a> examples for an illustration.

  Classical constraints are provided for convenience (see LocalConstraint,
  WorldConstraint and CameraConstraint) and new constraints can very easily be
  implemented.

  <h3>Derived classes</h3>

  The ManipulatedFrame class inherits Frame and implements a mouse motion
  convertion, so that a Frame (and hence an object) can be manipulated in the
  scene with the mouse.

  \nosubgrouping */
class Frame
{
public:
	Frame();

	/*! Virtual destructor. Empty. */
	virtual ~Frame() {}

	Frame(const Frame &frame);
	Frame &operator=(const Frame &frame);

public:
	/*! @name World coordinates position and orientation */
	//@{
	Frame(const vec3 &position, const quat &orientation);

	void setPosition(const vec3 &position);
	void setPosition(double x, double y, double z);
	void setPositionWithConstraint(vec3 &position);

	void setOrientation(const quat &orientation);
	void setOrientation(double q0, double q1, double q2, double q3);
	void setOrientationWithConstraint(quat &orientation);

	void setPositionAndOrientation(const vec3 &position,
		const quat &orientation);
	void setPositionAndOrientationWithConstraint(vec3 &position,
		quat &orientation);

	vec3 position() const;
	quat orientation() const;

	void getPosition(double &x, double &y, double &z) const;
	void getOrientation(double &q0, double &q1, double &q2, double &q3) const;
	//@}

public:
	/*! @name Local translation and rotation w/r reference Frame */
	//@{
	/*! Sets the translation() of the frame, locally defined with respect to the
	referenceFrame(). Emits the modified() signal.

	Use setPosition() to define the world coordinates position(). Use
	setTranslationWithConstraint() to take into account the potential constraint()
	of the Frame. */
	void setTranslation(const vec3 &translation) {
		t_ = translation;
		frameModified();
	}
	void setTranslation(double x, double y, double z);
	void setTranslationWithConstraint(vec3 &translation);

	/*! Set the current rotation quat. See rotation() and the different
	quat constructors. Emits the modified() signal. See also
	setTranslation() and setRotationWithConstraint(). */

	/*! Sets the rotation() of the Frame, locally defined with respect to the
	 referenceFrame(). Emits the modified() signal.

	 Use setOrientation() to define the world coordinates orientation(). The
	 potential constraint() of the Frame is not taken into account, use
	 setRotationWithConstraint() instead. */
	void setRotation(const quat &rotation) {
		q_ = rotation;
		frameModified();
	}
	void setRotation(double q0, double q1, double q2, double q3);
	void setRotationWithConstraint(quat &rotation);

	void setTranslationAndRotation(const vec3 &translation,
		const quat &rotation);
	void setTranslationAndRotationWithConstraint(vec3 &translation,
		quat &rotation);

	/*! Returns the Frame translation, defined with respect to the
	referenceFrame().

	Use position() to get the result in the world coordinates. These two values
	are identical when the referenceFrame() is \c NULL (default).

	See also setTranslation() and setTranslationWithConstraint(). */
	vec3 translation() const { return t_; }
	/*! Returns the Frame rotation, defined with respect to the referenceFrame().

	Use orientation() to get the result in the world coordinates. These two values
	are identical when the referenceFrame() is \c NULL (default).

	See also setRotation() and setRotationWithConstraint(). */

	/*! Returns the current quat orientation. See setRotation(). */
	quat rotation() const { return q_; }

	void getTranslation(double &x, double &y, double &z) const;
	void getRotation(double &q0, double &q1, double &q2, double &q3) const;
	//@}

public:
	/*! @name Frame hierarchy */
	//@{
	/*! Returns the reference Frame, in which coordinates system the Frame is
	defined.

	The translation() and rotation() of the Frame are defined with respect to the
	referenceFrame() coordinate system. A \c NULL referenceFrame() (default value)
	means that the Frame is defined in the world coordinate system.

	Use position() and orientation() to recursively convert values along the
	referenceFrame() chain and to get values expressed in the world coordinate
	system. The values match when the referenceFrame() is \c NULL.

	Use setReferenceFrame() to set this value and create a Frame hierarchy.
	Convenient functions allow you to convert 3D coordinates from one Frame to an
	other: see coordinatesOf(), localCoordinatesOf(), coordinatesOfIn() and their
	inverse functions.

	Vectors can also be converted using transformOf(), transformOfIn,
	localTransformOf() and their inverse functions. */
	const Frame *referenceFrame() const { return referenceFrame_; }
	void setReferenceFrame(const Frame *const refFrame);
	bool settingAsReferenceFrameWillCreateALoop(const Frame *const frame);
	//@}

	/*! @name Frame modification */
	//@{
	void translate(vec3 &t);
	void translate(const vec3 &t);
	// Some compilers complain about "overloading cannot distinguish from previous
	// declaration" Simply comment out the following method and its associated
	// implementation
	void translate(double x, double y, double z);
	void translate(double &x, double &y, double &z);

	void rotate(quat &q);
	void rotate(const quat &q);
	// Some compilers complain about "overloading cannot distinguish from previous
	// declaration" Simply comment out the following method and its associated
	// implementation
	void rotate(double q0, double q1, double q2, double q3);
	void rotate(double &q0, double &q1, double &q2, double &q3);

	void rotateAroundPoint(quat &rotation, const vec3 &point);
	void rotateAroundPoint(const quat &rotation, const vec3 &point);

	void alignWithFrame(const Frame *const frame, bool move = false,
		double threshold = 0.0);
	void projectOnLine(const vec3 &origin, const vec3 &direction);
	//@}

	/*! @name Coordinate system transformation of 3D coordinates */
	//@{
	vec3 coordinatesOf(const vec3 &src) const;
	vec3 inverseCoordinatesOf(const vec3 &src) const;
	vec3 localCoordinatesOf(const vec3 &src) const;
	vec3 localInverseCoordinatesOf(const vec3 &src) const;
	vec3 coordinatesOfIn(const vec3 &src, const Frame *const in) const;
	vec3 coordinatesOfFrom(const vec3 &src, const Frame *const from) const;

	void getCoordinatesOf(const double src[3], double res[3]) const;
	void getInverseCoordinatesOf(const double src[3], double res[3]) const;
	void getLocalCoordinatesOf(const double src[3], double res[3]) const;
	void getLocalInverseCoordinatesOf(const double src[3], double res[3]) const;
	void getCoordinatesOfIn(const double src[3], double res[3],
		const Frame *const in) const;
	void getCoordinatesOfFrom(const double src[3], double res[3],
		const Frame *const from) const;
	//@}

	/*! @name Coordinate system transformation of vectors */
	// A frame is as a new coordinate system, defined with respect to a reference
	// frame (the world coordinate system by default, see the "Composition of
	// frame" section).

	// The transformOf() (resp. inverseTransformOf()) functions transform a 3D
	// vector from (resp. to) the world coordinates system. This section defines
	// the 3D vector transformation functions. See the Coordinate system
	// transformation of 3D points above for the transformation of 3D points. The
	// difference between the two sets of functions is simple: for vectors, only
	// the rotational part of the transformations is taken into account, while
	// translation is also considered for 3D points.

	// The length of the resulting transformed vector is identical to the one of
	// the source vector for all the described functions.

	// When local is prepended to the names of the functions, the functions simply
	// transform from (and to) the reference frame.

	// When In (resp. From) is appended to the names, the functions transform from
	// (resp. To) the frame that is given as an argument. The frame does not need
	// to be in the same branch or the hierarchical tree, and can be \c NULL (the
	// world coordinates system).

	// Combining any of these functions with its inverse (in any order) leads to
	// the identity.
	//@{
	vec3 transformOf(const vec3 &src) const;
	vec3 inverseTransformOf(const vec3 &src) const;
	vec3 localTransformOf(const vec3 &src) const;
	vec3 localInverseTransformOf(const vec3 &src) const;
	vec3 transformOfIn(const vec3 &src, const Frame *const in) const;
	vec3 transformOfFrom(const vec3 &src, const Frame *const from) const;

	void getTransformOf(const double src[3], double res[3]) const;
	void getInverseTransformOf(const double src[3], double res[3]) const;
	void getLocalTransformOf(const double src[3], double res[3]) const;
	void getLocalInverseTransformOf(const double src[3], double res[3]) const;
	void getTransformOfIn(const double src[3], double res[3],
		const Frame *const in) const;
	void getTransformOfFrom(const double src[3], double res[3],
		const Frame *const from) const;
	//@}

	  /*! @name Constraint on the displacement */
  //@{
  /*! Returns the current constraint applied to the Frame.

  A \c NULL value (default) means that no Constraint is used to filter Frame
  translation and rotation. See the Constraint class documentation for details.

  You may have to use a \c dynamic_cast to convert the result to a Constraint
  derived class. */
	Constraint *constraint() const { return constraint_; }
	/*! Sets the constraint() attached to the Frame.

	A \c NULL value means no constraint. The previous constraint() should be
	deleted by the calling method if needed. */
	void setConstraint(Constraint *const constraint) { constraint_ = constraint; }
	//@}


	/*! @name Associated matrices */
	//@{
public:
	mat4 matrix() const;
	mat4 worldMatrix() const;

	void setFromMatrix(const mat4& m);

	//@}

	/*! @name Inversion of the transformation */
	//@{
	Frame inverse() const;
	/*! Returns the inverse() of the Frame world transformation.

	The orientation() of the new Frame is the quat::inverse() of the
	original orientation. Its position() is the negated and inverse rotated image
	of the original position.

	The result Frame has a \c NULL referenceFrame() and a \c NULL constraint().

	Use inverse() for a local (i.e. with respect to referenceFrame())
	transformation inverse. */
	Frame worldInverse() const {
		return Frame(-(orientation().inverse_rotate(position())),
			orientation().inverse());
	}
	
public:
	void addObserver(FrameObserver* obs) { observers_.insert(obs); }
	void removeObserver(FrameObserver* obs) { observers_.erase(obs); }

protected:
	// Should be called whenever the position() or orientation() of the Frame is modified.
	// Then all observers will be notified the modification of the frame.
	void frameModified();

private:
	std::set< FrameObserver* >	observers_;

private:
	// P o s i t i o n   a n d   o r i e n t a t i o n
	vec3 t_;
	quat q_;

	// C o n s t r a i n t s
	Constraint *constraint_;

	// F r a m e   c o m p o s i t i o n
	const Frame *referenceFrame_;
};



#endif // QGLVIEWER_FRAME_H