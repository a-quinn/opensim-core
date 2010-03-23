// InducedAccelerations.cpp
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//	AUTHOR: Ajay Seth
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/* Copyright (c)  2009 Stanford University
* Use of the OpenSim software in source form is permitted provided that the following
* conditions are met:
* 	1. The software is used only for non-commercial research and education. It may not
*     be used in relation to any commercial activity.
* 	2. The software is not distributed or redistributed.  Software distribution is allowed 
*     only through https://simtk.org/home/opensim.
* 	3. Use of the OpenSim software or derivatives must be acknowledged in all publications,
*      presentations, or documents describing work in which OpenSim or derivatives are used.
* 	4. Credits to developers may not be removed from executables
*     created from modifications of the source.
* 	5. Modifications of source code must retain the above copyright notice, this list of
*     conditions and the following disclaimer. 
* 
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
*  EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
*  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
*  SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
*  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
*  TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; 
*  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
*  OR BUSINESS INTERRUPTION) OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY
*  WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

//=============================================================================
// INCLUDES
//=============================================================================
#include <iostream>
#include <string>
#include <OpenSim/Common/IO.h>
#include <OpenSim/Common/FunctionSet.h>
#include <OpenSim/Simulation/Model/Model.h>
#include <OpenSim/Simulation/Model/BodySet.h>
#include <OpenSim/Simulation/Model/CoordinateSet.h>
#include <OpenSim/Simulation/Model/ForceSet.h>
#include <OpenSim/Simulation/Model/PrescribedForce.h>
#include <OpenSim/Simulation/SimbodyEngine/SimbodyEngine.h>
#include <OpenSim/Simulation/SimbodyEngine/RollingOnSurfaceConstraint.h>
#include "InducedAccelerations.h"

using namespace OpenSim;
using namespace std;

//=============================================================================
// CONSTANTS
//=============================================================================
#define CENTER_OF_MASS_NAME string("CENTER_OF_MASS")

//=============================================================================
// CONSTRUCTOR(S) AND DESTRUCTOR
//=============================================================================
//_____________________________________________________________________________
/**
 * Destructor.
 * Delete any variables allocated using the "new" operator.  You will not
 * necessarily have any of these.
 */
InducedAccelerations::~InducedAccelerations()
{
	delete &_coordSet;
	delete &_bodySet;
}
//_____________________________________________________________________________
/*
 * Construct an InducedAccelerations instance.
 *
 * @param aModel Model for which the analysis is to be run.
 */
InducedAccelerations::InducedAccelerations(Model *aModel) :
	Analysis(aModel),
	_coordNames(_coordNamesProp.getValueStrArray()),
	_bodyNames(_bodyNamesProp.getValueStrArray()),
	_constraintSetProp(PropertyObj("", ConstraintSet())),
	_constraintSet((ConstraintSet&)_constraintSetProp.getValueObj()),
	_forceThreshold(_forceThresholdProp.getValueDbl()),
	_computePotentialsOnly(_computePotentialsOnlyProp.getValueBool()),
	_bodySet(*new BodySet()),
	_coordSet(*new CoordinateSet())
{
	// make sure members point to NULL if not valid. 
	setNull();
	if(_model==NULL) return;

	// DESCRIPTION
	constructDescription();
}
//_____________________________________________________________________________
/*
 * Construct an object from file.
 *
 * The object is constructed from the root element of the XML document.
 * The type of object is the tag name of the XML root element.
 *
 * @param aFileName File name of the document.
 */
InducedAccelerations::InducedAccelerations(const std::string &aFileName):
	Analysis(aFileName, false),
	_coordNames(_coordNamesProp.getValueStrArray()),
	_bodyNames(_bodyNamesProp.getValueStrArray()),
	_constraintSetProp(PropertyObj("", ConstraintSet())),
	_constraintSet((ConstraintSet&)_constraintSetProp.getValueObj()),
	_forceThreshold(_forceThresholdProp.getValueDbl()),
	_computePotentialsOnly(_computePotentialsOnlyProp.getValueBool()),
	_bodySet(*new BodySet()),
	_coordSet(*new CoordinateSet())
{
	setNull();

	// Read properties from XML
	updateFromXMLNode();
}

// Copy constrctor and virtual copy 
//_____________________________________________________________________________
/*
 * Copy constructor.
 *
 */
InducedAccelerations::InducedAccelerations(const InducedAccelerations &aInducedAccelerations):
	Analysis(aInducedAccelerations),
	_coordNames(_coordNamesProp.getValueStrArray()),
	_bodyNames(_bodyNamesProp.getValueStrArray()),
	_constraintSetProp(PropertyObj("", ConstraintSet())),
	_constraintSet((ConstraintSet&)_constraintSetProp.getValueObj()),
	_forceThreshold(_forceThresholdProp.getValueDbl()),
	_computePotentialsOnly(_computePotentialsOnlyProp.getValueBool()),
	_bodySet(*new BodySet()),
	_coordSet(*new CoordinateSet())
{
	setNull();
	// COPY TYPE AND NAME
	*this = aInducedAccelerations;
}
//_____________________________________________________________________________
/**
 * Clone
 *
 */
Object* InducedAccelerations::copy() const
{
	InducedAccelerations *object = new InducedAccelerations(*this);
	return(object);

}
//=============================================================================
// OPERATORS
//=============================================================================
//-----------------------------------------------------------------------------
// ASSIGNMENT
//-----------------------------------------------------------------------------
//_____________________________________________________________________________
/*
 * Assign this object to the values of another.
 *
 * @return Reference to this object.
 */
InducedAccelerations& InducedAccelerations::
operator=(const InducedAccelerations &aInducedAccelerations)
{
	// Base Class
	Analysis::operator=(aInducedAccelerations);

	// Member Variables
	_coordNames = aInducedAccelerations._coordNames;
	_bodyNames = aInducedAccelerations._bodyNames;
	_constraintSet = aInducedAccelerations._constraintSet;
	_forceThreshold = aInducedAccelerations._forceThreshold;
	_computePotentialsOnly = aInducedAccelerations._computePotentialsOnly;
	_includeCOM = aInducedAccelerations._includeCOM;
	return(*this);
}

//_____________________________________________________________________________
/**
 * SetNull().
 */
void InducedAccelerations::setNull()
{
	setType("InducedAccelerations");
	setupProperties();

	_forceThreshold = 6.00;
	_coordNames.setSize(0);
	_bodyNames.setSize(1);
	_bodyNames[0] = CENTER_OF_MASS_NAME;
	_computePotentialsOnly = false;
	// Analysis does not own contents of these sets
	_coordSet.setMemoryOwner(false);
	_bodySet.setMemoryOwner(false);
}
//_____________________________________________________________________________
/*
 * Set up the properties for your analysis.
 *
 * You should give each property a meaningful name and an informative comment.
 * The name you give each property is the tag that will be used in the XML
 * file.  The comment will appear before the property in the XML file.
 * In addition, the comments are used for tool tips in the OpenSim GUI.
 *
 * All properties are added to the property set.  Once added, they can be
 * read in and written to file.
 */
void InducedAccelerations::setupProperties()
{
	_coordNamesProp.setName("coordinate_names");
	_coordNamesProp.setComment("Names of the coordinates for which to compute induced accelerations."
		"The key word 'All' indicates that the analysis should be performed for all coordinates.");
	_propertySet.append(&_coordNamesProp);

	_bodyNamesProp.setName("body_names");
	_bodyNamesProp.setComment("Names of the bodies for which to compute induced accelerations."
		"The key word 'All' indicates that the analysis should be performed for all bodies."
		"Use 'center_of_mass' to indicate the induced accelerations of the system center of mass.");
	_propertySet.append(&_bodyNamesProp);

	_constraintSetProp.setName("ConstraintSet");
	_constraintSetProp.setComment("Specify the constraints used to replace ground contact."
								  "Currently, RollingOnSurfaceConstraints are supported ");
	_propertySet.append(&_constraintSetProp);

	_forceThresholdProp.setName("force_threshold");
	_forceThresholdProp.setComment("The minimum amount of contact force (N) that is sufficient to be replaced with a constraint.");
	_propertySet.append(&_forceThresholdProp);

	_computePotentialsOnlyProp.setName("compute_potentials_only");
	_computePotentialsOnlyProp.setComment("Only compute the potential (accelertion/force) of a muscle to accelerate the model.");
	_propertySet.append(&_computePotentialsOnlyProp);
}

//=============================================================================
// CONSTRUCTION METHODS
//=============================================================================
//_____________________________________________________________________________
/**
 * Construct a description for the body kinematics files.
 */
void InducedAccelerations::constructDescription()
{
	string descrip;

	descrip = "\nThis file contains accelerations of coordinates or bodies.\n";
	descrip += "\nUnits are S.I. units (seconds, meters, Newtons, ...)";
	if(getInDegrees()) {
		descrip += "\nAngles are in degrees.";
	} else {
		descrip += "\nAngles are in radians.";
	}
	descrip += "\n\n";

	setDescription(descrip);
	assembleContributors();
}

//_____________________________________________________________________________
/**
 * Assemble the list of contributors for induced acceleration analysis
 */
void InducedAccelerations:: assembleContributors()
{
	Array<string> contribs;
	if (!_computePotentialsOnly)
		contribs.append("total");

	const Set<Actuator> &actuatorSet = _model->getActuators();

	//Do the analysis on the bodies that are in the indices list
	for(int i=0; i< actuatorSet.getSize(); i++) {
		contribs.append(actuatorSet.get(i).getName()) ;
	}
 
	contribs.append("gravity");
	contribs.append("velocity");

	_contributors = contribs;
}

/**
 * Construct column labels for the output results.
 *
 * For analyses that run during a simulation, the first column is 
 * always time.  For the purpose of example, the code below adds labels
 * for each contributor
 * This method needs to be called as necessary to update the column labels.
 */
Array<string> InducedAccelerations:: constructColumnLabelsForCoordinate()
{
	Array<string> labels;
	labels.append("time");
	labels.append(_contributors);

	return labels;
}

/**
 * Construct column labels for the body acceleration results.
 *
 * For analyses that run during a simulation, the first column is 
 * always time.  For the purpose of example, the code below adds labels
 * appropriate for recording the translation and orientation of the 
 * desired body.
 *
 * This method needs to be called as necessary to update the column labels.
 */
Array<string> InducedAccelerations:: constructColumnLabelsForBody()
{
	// Get the main headings for all the contributors
	Array<string> contributors = constructColumnLabelsForCoordinate();
	Array<string> labels;

	// first label is time not a contributor
	labels.append(contributors[0]);
	for(int i=1; i<contributors.getSize(); i++) {
		labels.append(contributors[i] + "_X");
		labels.append(contributors[i] + "_Y");
		labels.append(contributors[i] + "_Z");
		labels.append(contributors[i] + "_Ox");
		labels.append(contributors[i] + "_Oy");
		labels.append(contributors[i] + "_Oz");
	}
	return labels;
}

Array<string> InducedAccelerations:: constructColumnLabelsForCOM()
{
	// Get the main headings for all the contributors
	Array<string> contributors = constructColumnLabelsForCoordinate();
	Array<string> labels;

	// first label is time not a contributor
	labels.append(contributors[0]);
	for(int i=1; i<contributors.getSize(); i++) {
		// ADD CONTRIBUTORS TO THE ACCELERATION OF THE WHOLE BODY
		labels.append(contributors[i] + "_X");
		labels.append(contributors[i] + "_Y");
		labels.append(contributors[i] + "_Z");
	}
	return labels;
}


//_____________________________________________________________________________
/**
 * Set up storage objects.
 *
 * In general, the storage objects in your analysis are used to record
 * the results of your analysis and write them to file.  You will often
 * have a number of storage objects, each for recording a different
 * kind of result.
 */
void InducedAccelerations::setupStorage()
{
	const CoordinateSet& modelCoordSet = _model->getCoordinateSet();
	int nc = _coordNames.getSize();

	// Get the indices of the coordinates we are interested in
	_storeInducedAccelerations.setSize(0);
	_coordSet.setSize(0);
	if(nc && (IO::Uppercase(_coordNames.get(0)) == "ALL")) {
		nc = modelCoordSet.getSize();
		for(int i=0; i<nc; i++)
			_coordSet.append(&modelCoordSet.get(i));
	}
	else{
		for(int i=0; i<nc; i++){
			int index = modelCoordSet.getIndex(_coordNames[i]);
			if(index<0) throw Exception("InducedAcceleration: ERR- Could not find coordinate '"+_coordNames[i],__FILE__,__LINE__);
			_coordSet.append(&modelCoordSet.get(index));
		}
	}

	// Setup storage object or each coordinate
	nc = _coordSet.getSize();
	_coordIndAccs.setSize(0);
	Array<string> coordAccLabels = constructColumnLabelsForCoordinate();
	for(int i=0; i<nc; i++){
		_storeInducedAccelerations.append(new Storage(1000));
		_storeInducedAccelerations[i]->setName(_coordSet.get(i).getName());
		_storeInducedAccelerations[i]->setDescription(getDescription());
		_storeInducedAccelerations[i]->setColumnLabels(coordAccLabels);
		_coordIndAccs.append(new Array<double>(0, coordAccLabels.getSize()));
	}

	// Now get the bodies that we are interested, including system center of mass
	const BodySet& modelBodySet = _model->getBodySet();
	int nb = _bodyNames.getSize();

	_bodySet.setSize(0);

	if(nb && (IO::Uppercase(_bodyNames.get(0)) == "ALL")) {
		nb = modelBodySet.getSize();
		for(int i=0; i<nb; i++)
			_bodySet.append(&modelBodySet.get(i));
	}
	else{
		for(int i=0; i<nb; i++){
			if(IO::Uppercase(_bodyNames.get(i)) == CENTER_OF_MASS_NAME)
				_includeCOM = true;
			else{
				int bi =  modelBodySet.getIndex(_bodyNames[i]);
				if(bi<0) throw Exception("InducedAcceleration: ERR- Could not find body '"+_bodyNames[i],__FILE__,__LINE__);
				_bodySet.append(&modelBodySet.get(bi));
			}
		}
	}

	// Setup storage object for bodies
	nb = _bodySet.getSize();
	_bodyIndAccs.setSize(0);
	Array<string> bodyAccLabels = constructColumnLabelsForBody();
	for(int i=0; i<nb; i++){
		_storeInducedAccelerations.append(new Storage(1000));
		cout << "Body " << i << " in _bodySet: " << _bodySet.get(i).getName() << endl;
		_storeInducedAccelerations[nc+i]->setName(_bodySet.get(i).getName());
		_storeInducedAccelerations[nc+i]->setDescription(getDescription());
		_storeInducedAccelerations[nc+i]->setColumnLabels(bodyAccLabels);
		_bodyIndAccs.append(new Array<double>(0, bodyAccLabels.getSize()));
	}

	if(_includeCOM){
		Array<string> comAccLabels = constructColumnLabelsForCOM();
		_comIndAccs.setSize(0);
		_storeInducedAccelerations.append(new Storage(1000, CENTER_OF_MASS_NAME));
		_storeInducedAccelerations[nc+nb]->setDescription(getDescription());
		_storeInducedAccelerations[nc+nb]->setColumnLabels(comAccLabels);
	}

	_coordSet.setMemoryOwner(false);
	_bodySet.setMemoryOwner(false);
}


//=============================================================================
// GET AND SET
//=============================================================================
//_____________________________________________________________________________
/**
 * Set the model for which this analysis is to be run.
 *
 * Sometimes the model on which an analysis should be run is not available
 * at the time an analysis is created.  Or, you might want to change the
 * model.  This method is used to set the model on which the analysis is
 * to be run.
 *
 * @param aModel Model pointer
 */
void InducedAccelerations::setModel(Model &aModel)
{
	// SET THE MODEL for this analysis to be a copy
	Analysis::setModel(aModel); //*dynamic_cast<Model*>(aModel.copy()));
}


//=============================================================================
// ANALYSIS
//=============================================================================
//_____________________________________________________________________________
/**
 * Compute and record the results.
 *
 * This method, for the purpose of example, records the position and
 * orientation of each body in the model.  You will need to customize it
 * to perform your analysis.
 *
 * @param aT Current time in the simulation.
 * @param aX Current values of the controls.
 * @param aY Current values of the states: includes generalized coords and speeds
 */
int InducedAccelerations::record(const SimTK::State& s)
{
	int nu = _model->getNumSpeeds();
	double aT = s.getTime();
	cout << "time = " << aT << endl;

	SimTK::Vector Q = s.getQ();

	// Reset Accelerations for coordinates at this time step
	for(int i=0;i<_coordSet.getSize();i++) {
		_coordIndAccs[i]->setSize(0);
	}

	// Reset Accelerations for bodies at this time step
	for(int i=0;i<_bodySet.getSize();i++) {
		_bodyIndAccs[i]->setSize(0);
	}

	// Reset Accelerations for system center of mass at this time step
	_comIndAccs.setSize(0);

	SimTK::State s_analysis = _model->getMultibodySystem().updDefaultState();
	_model->initStateWithoutRecreatingSystem(s_analysis);
	// Just need to set current time and position to determine state of constraints
	s_analysis.setTime(aT);
	s_analysis.setQ(Q);

	// Check the external forces and determine if contact constraints should be applied at this time
	// and turn constraint on if it should be.
	Array<bool> constraintOn = applyContactConstraintAccordingToExternalForces(s_analysis);

	// Hang on to a state that has the right flags for contact constraints turned on/off
	_model->setDefaultsFromState(s_analysis);
	// Use this state for the remainder of this step (record)
	s_analysis = _model->getMultibodySystem().realizeTopology();
	// DO NOT recreate the system, will lose location of constraint
	_model->initStateWithoutRecreatingSystem(s_analysis);

	// Cycle through the force contributors to the system acceleration
	for(int c=0; c< _contributors.getSize(); c++){			
		//cout << "Solving for contributor: " << _contributors[c] << endl;
		// Need to be at the dynamics stage to disable a force
		_model->getMultibodySystem().realize(s_analysis, SimTK::Stage::Dynamics);
		
		if(_contributors[c] == "total"){
			// Set gravity ON
			_model->getGravityForce().enable(s_analysis);

			//Use same conditions on constraints
			s_analysis.setTime(aT);
			// Set the configuration (gen. coords and speeds) of the model.
			s_analysis.setQ(Q);
			s_analysis.setU(s.getU());
			s_analysis.setZ(s.getZ());

			//Make sure all the actuators are on!
			for(int f=0; f<_model->getActuators().getSize(); f++){
				_model->updActuators().get(f).setDisabled(s_analysis, false);
			}

			// Get to  the point where we can evaluate unilateral constraint conditions
			_model->getMultibodySystem().realize(s_analysis, SimTK::Stage::Acceleration);
	
			for(int i=0; i<constraintOn.getSize(); i++) {
				_constraintSet.get(i).setDisabled(s_analysis, !constraintOn[i]);
				// Make sure we stay at Dynamics so each constraint can evaluate its conditions
				_model->getMultibodySystem().realize(s_analysis, SimTK::Stage::Acceleration);
			}

			// This should also push changes to defaults for unilateral conditions
			_model->setDefaultsFromState(s_analysis);

		}
		else if(_contributors[c] == "gravity"){
			// Set gravity ON
			_model->updUserForceSubsystem().setForceIsDisabled(s_analysis, _model->getGravityForce().getForceIndex(), false);

			//s_analysis = _model->initSystem();
			s_analysis.setTime(aT);
			s_analysis.setQ(Q);

			// zero velocity
			s_analysis.setU(SimTK::Vector(nu,0.0));
			s_analysis.setZ(s.getZ());

			// disable actuator forces
			for(int f=0; f<_model->getActuators().getSize(); f++){
				_model->updActuators().get(f).setDisabled(s_analysis, true);
			}
		}
		else if(_contributors[c] == "velocity"){		
			// Set gravity off
			_model->updUserForceSubsystem().setForceIsDisabled(s_analysis, _model->getGravityForce().getForceIndex(), true);

			s_analysis.setTime(aT);
			s_analysis.setQ(Q);

			// non-zero velocity
			s_analysis.setU(s.getU());
			s_analysis.setZ(s.getZ());
			
			// zero actuator forces
			for(int f=0; f<_model->getActuators().getSize(); f++){
				_model->updActuators().get(f).setDisabled(s_analysis, true);
			}
			// Set the configuration (gen. coords and speeds) of the model.
			_model->getMultibodySystem().realize(s_analysis, SimTK::Stage::Velocity);
		}
		else{ //The rest are actuators		
			// Set gravity ON
			_model->updUserForceSubsystem().setForceIsDisabled(s_analysis, _model->getGravityForce().getForceIndex(), true);

			// zero actuator forces
			for(int f=0; f<_model->getActuators().getSize(); f++){
				_model->updActuators().get(f).setDisabled(s_analysis, true);
			}

			//s_analysis = _model->initSystem();
			s_analysis.setTime(aT);
			s_analysis.setQ(Q);

			// zero velocity
			SimTK::Vector U(nu,0.0);
			s_analysis.setU(U);
			s_analysis.setZ(s.getZ());
			// light up the one actuator who's contribution we are looking for
			int ai = _model->getActuators().getIndex(_contributors[c]);
			if(ai<0)
				throw Exception("InducedAcceleration: ERR- Could not find actuator '"+_contributors[c],__FILE__,__LINE__);
			
			Actuator &actuator = _model->getActuators().get(ai);
			actuator.setDisabled(s_analysis, false);
			
			if(_computePotentialsOnly){
				actuator.setIsControlled(false);
				actuator.setForce(s_analysis, 1.0);
			}
			else{
				actuator.setIsControlled(true);
			}

			// Set the configuration (gen. coords and speeds) of the model.
			_model->getMultibodySystem().realize(s_analysis, SimTK::Stage::Model);
			_model->getMultibodySystem().realize(s_analysis, SimTK::Stage::Velocity);

		}// End of if to select contributor 

		//cout << "Constraint 0 should be  " << constraintOn[0] << " and is " <<  (_constraintSet[0].isDisabled(s_analysis) ? "off" : "on") << endl;
		//cout << "Constraint 1 should be  " << constraintOn[1] << " and is " <<  (_constraintSet[1].isDisabled(s_analysis) ? "off" : "on") << endl;

		// After setting the state of the model and applying forces
		// Compute the derivative of the multibody system (speeds and accelerations)
		_model->getMultibodySystem().realize(s_analysis, SimTK::Stage::Acceleration);

		// Sanity check that constraints hasn't totally changed the configuration of the model
		double error = (Q-s_analysis.getQ()).norm();

		// Report reaction forces for debugging
		/*
		SimTK::Vector_<SimTK::SpatialVec> constraintBodyForces(_constraintSet.getSize());
		SimTK::Vector mobilityForces(0);

		for(int i=0; i<constraintOn.getSize(); i++) {
			if(constraintOn[i])
				_constraintSet.get(i).calcConstraintForces(s_analysis, constraintBodyForces, mobilityForces);
		}*/

		// VARIABLES
		SimTK::Vec3 vec,angVec;

		// Get Accelerations for kinematics of bodies
		for(int i=0;i<_coordSet.getSize();i++) {
			double acc = _coordSet.get(i).getAccelerationValue(s_analysis);

			if(getInDegrees()) 
				acc *= SimTK_RADIAN_TO_DEGREE;	
			_coordIndAccs[i]->append(1, &acc);
		}

		//cout << "Input Body Names: "<< _bodyNames << endl;

		// Get Accelerations for kinematics of bodies
		for(int i=0;i<_bodySet.getSize();i++) {
			Body &body = _bodySet.get(i);
			//cout << "Body Name: "<< body->getName() << endl;
			SimTK::Vec3 com(0);
			// Get the center of mass location for this body
			body.getMassCenter(com);
			
			// Get the body acceleration
			_model->getSimbodyEngine().getAcceleration(s_analysis, body, com, vec);
			_model->getSimbodyEngine().getAngularAcceleration(s_analysis, body, angVec);	

			// CONVERT TO DEGREES?
			if(getInDegrees()) 
				angVec *= SimTK_RADIAN_TO_DEGREE;	

			// FILL KINEMATICS ARRAY
			_bodyIndAccs[i]->append(3, &vec[0]);
			_bodyIndAccs[i]->append(3, &angVec[0]);
		}

		// Get Accelerations for kinematics of COM
		if(_includeCOM){
			// Get the body acceleration in ground
			vec = _model->getMultibodySystem().getMatterSubsystem().calcSystemMassCenterAccelerationInGround(s_analysis);

			// FILL KINEMATICS ARRAY
			_comIndAccs.append(3, &vec[0]);
		}
	} // End cycling through contributors at this time step

	// Set the accelerations of coordinates into their storages
	int nc = _coordSet.getSize();
	for(int i=0; i<nc; i++) {
		_storeInducedAccelerations[i]->append(aT, _coordIndAccs[i]->getSize(),&(_coordIndAccs[i]->get(0)));
	}

	// Set the accelerations of bodies into their storages
	int nb = _bodySet.getSize();
	for(int i=0; i<nb; i++) {
		_storeInducedAccelerations[nc+i]->append(aT, _bodyIndAccs[i]->getSize(),&(_bodyIndAccs[i]->get(0)));
	}

	// Set the accelerations of system center of mass into a storage
	_storeInducedAccelerations[nc+nb]->append(aT, _comIndAccs.getSize(), &_comIndAccs[0]);

	return(0);
}

/**
 * This method is called at the beginning of an analysis so that any
 * necessary initializations may be performed.
 *
 * @param s SimTK:State
 */
void InducedAccelerations::initialize(const SimTK::State& s)
{
	// Go forward with a copy of the model so Analysis can add to model if necessary
	_model = dynamic_cast<Model*>(_model->copy());

	SimTK::State s_copy = s;
	double time = s_copy.getTime();

	_externalForces.setSize(0);
	_constraints.setSize(0);

	//Only add constraint set if constraint type for the analysis is Roll
	for(int i=0; i<_constraintSet.getSize(); i++){
		Constraint* contactConstraint = &_constraintSet.get(i);
		if(contactConstraint)
			_model->updConstraintSet().append(contactConstraint);
	}

	// Create a set of constraints used to model contact with the ground
	// based on external forces (PrescribedForces) applied to the model
	for(int i=0; i < _model->getForceSet().getSize(); i++){
		PrescribedForce *exf = dynamic_cast<PrescribedForce *>(&_model->getForceSet().get(i));
		if(exf){
			addContactConstraintFromExternalForce(exf);
			exf->setDisabled(s_copy, true);
		}
	}

	// Get value for gravity
	_gravity = _model->getGravity();

	SimTK::State &s_analysis =_model->initSystem();

	// UPDATE VARIABLES IN THIS CLASS
	constructDescription();
	setupStorage();
}

//_____________________________________________________________________________
/**
 * This method is called at the beginning of an analysis so that any
 * necessary initializations may be performed.
 *
 * This method is meant to be called at the begining of an integration in
 * Model::integBeginCallback() and has the same argument list.
 *
 * @param s SimTK:State
 *
 * @return -1 on error, 0 otherwise.
 */
int InducedAccelerations::begin(const SimTK::State &s)
{
	if(!proceed()) return(0);

	initialize(s);

	// RESET STORAGES
	for(int i = 0; i<_storeInducedAccelerations.getSize(); i++){
		_storeInducedAccelerations[i]->reset(s.getTime());
	}

	cout << "Performing Induced Accelerations Analysis" << endl;

	// RECORD
	int status = 0;
	status = record(s);

	return(status);
}
//_____________________________________________________________________________
/**
 * This method is called to perform the analysis.  It can be called during
 * the execution of a forward integrations or after the integration by
 * feeding it the necessary data.
 *
 * When called during an integration, this method is meant to be called in
 * Model::integStepCallback(), which has the same argument list.
 *
 * @param s, state
 * @param stepNumber
 *
 * @return -1 on error, 0 otherwise.
 */
int InducedAccelerations::step(const SimTK::State &s, int stepNumber)
{
	if(proceed(stepNumber) && getOn())
		record(s);

	return(0);
}
//_____________________________________________________________________________
/**
 * This method is called at the end of an analysis so that any
 * necessary finalizations may be performed.
 *
 * This method is meant to be called at the end of an integration in
 * Model::integEndCallback() and has the same argument list.
 *
 * @param State
 *
 * @return -1 on error, 0 otherwise.
 */
int InducedAccelerations::end(const SimTK::State &s)
{
	if(!proceed()) return(0);

	record(s);

	return(0);
}




//=============================================================================
// IO
//=============================================================================
//_____________________________________________________________________________
/**
 * Print results.
 * 
 * The file names are constructed as
 * aDir + "/" + aBaseName + "_" + ComponentName + aExtension
 *
 * @param aDir Directory in which the results reside.
 * @param aBaseName Base file name.
 * @param aDT Desired time interval between adjacent storage vectors.  Linear
 * interpolation is used to print the data out at the desired interval.
 * @param aExtension File extension.
 *
 * @return 0 on success, -1 on error.
 */
int InducedAccelerations::
printResults(const string &aBaseName,const string &aDir,double aDT,
				 const string &aExtension)
{
	// Write out induced accelerations for all kinematic variables
	for(int i = 0; i < _storeInducedAccelerations.getSize(); i++){
		_storeInducedAccelerations[i]->scaleTime(_model->getTimeNormConstant());
		Storage::printResult(_storeInducedAccelerations[i],aBaseName+"_"
			                   +getName()+"_"+_storeInducedAccelerations[i]->getName(),aDir,aDT,aExtension);
	}
	
	return(0);
}


void InducedAccelerations::addContactConstraintFromExternalForce(PrescribedForce *externalForce)
{
	_externalForces.append(externalForce);
}

Array<bool> InducedAccelerations::applyContactConstraintAccordingToExternalForces(SimTK::State &s)
{
	Array<bool> constraintOn(false, _constraintSet.getSize());
	//int nc = 0;
	double t = s.getTime();

	for(int i=0; i<_externalForces.getSize(); i++){
		PrescribedForce *exf = _externalForces[i];
		SimTK::Vec3 point, force, gpoint;

		force = exf->getForceAtTime(t);
		
		// If the applied force is "significant" replace it with a constraint
		if (force.norm() > _forceThreshold){
			// turn on the constraint
			_constraintSet.get(i).setDisabled(s, false);
			// return the state of the constraint
			constraintOn[i] = true;

			// get the point of contact from applied external force
			point = exf->getPointAtTime(t);

			// set the point on the constraint
			((RollingOnSurfaceConstraint *)(&_constraintSet.get(i)))->setContactPointOnSurfaceBody(s, point);
		}
		else{
			// turn on the constraint
			_constraintSet.get(i).setDisabled(s, true);
			// return the state of the constraint
			constraintOn[i] = false;
		}
	}

	return constraintOn;
}