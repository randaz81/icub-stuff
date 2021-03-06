// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-
// Copyright (C) 2015 Istituto Italiano di Tecnologia - iCub Facility
// Author: Marco Randazzo <marco.randazzo@iit.it>
// CopyPolicy: Released under the terms of the GNU GPL v2.0.

// *******************************
// THIS MODULE IS EXPERIMENTAL!!!!
// *******************************

#include <stdio.h>
#include <yarp/os/Network.h>
#include <yarp/os/Property.h>
#include <yarp/os/Time.h>
#include <yarp/dev/PolyDriver.h>
#include <yarp/os/BufferedPort.h>
#include <yarp/os/RFModule.h>
#include <yarp/os/Module.h>
#include <yarp/os/Log.h>
#include <yarp/os/LogStream.h>
#include <string.h>
#include <yarp/dev/ControlBoardInterfaces.h>

//default rate for the control loop
const int CONTROL_PERIOD=10;

using namespace yarp::os;
using namespace yarp::dev;

class VergenceModule: public RFModule {
private:
    PolyDriver driver;
    char partName[255];
    IControlMode2 *icmd;
    IPositionDirect *idir;
    yarp::os::Port inport;
    yarp::os::Port outport;
    Port rpc_port;
    bool reset_offset;
    double initial_offset[6];

public:
    virtual bool respond(const yarp::os::Bottle &command, yarp::os::Bottle &reply)
    {
        reply.clear(); 
        if (command.get(0).isString())
        {
            if (command.get(0).asString()=="help")
            {
                reply.addVocab(Vocab::encode("many"));
                reply.addString("Available commands:");
                reply.addString("currently nothing");
                return true;
            }
            else if (command.get(0).asString()=="***")
            {
                return true;
            }
        }
        reply.addString("Unknown command");
        return true;
    }
  
    virtual bool configure(yarp::os::ResourceFinder &rf)
    {
        Property options;
        options.fromString(rf.toString());
        char robotName[255];
        Bottle *jointsList=0;
        std::string moduleName = "torqueObserver";
        Time::turboBoost();

        reset_offset = true;
        for (int i=0; i<6; i++) initial_offset[i]=0.0;

       /* options.put("device", "remote_controlboard");
        if(options.check("robot"))
            strncpy(robotName, options.find("robot").asString().c_str(),sizeof(robotName));
        else
            strncpy(robotName, "icub", sizeof(robotName));

        if (options.check("name"))
        {
            moduleName = options.find("name").asString();
        }

        if(options.check("part"))
        {
            sprintf(partName, "%s", options.find("part").asString().c_str());

            sprintf(tmp, "/%s/%s/%s/client", moduleName.c_str(), robotName, partName);
            options.put("local",tmp);
        
            sprintf(tmp, "/%s/%s", robotName, partName);
            options.put("remote", tmp);
        
            sprintf(tmp, "/%s/%s/rpc", moduleName.c_str(), partName);
            rpc_port.open(tmp);
            
            options.put("carrier", "tcp");
            
            attach(rpc_port);
        }
        else
        {
            yError("Please specify part (e.g. --part head)\n");
            return false;
        }

        //opening the device driver
        if (!driver.open(options))
        {
            yError("Error opening device, check parameters\n");
            return false;
        }

        bool ret = true;
        idir = 0;
        icmd = 0;
        ret &= driver.view(idir);
        ret &= driver.view(icmd);
      */
        char pin[255];
        sprintf(pin, "/%s/torque:i", moduleName.c_str());
        inport.open(pin);
        char pout[255];
        sprintf(pout, "/%s/torque:o", moduleName.c_str());
        outport.open(pout);

        bool b1 = yarp::os::Network::connect("/icub/left_leg/analog:o",pin);
        bool b2 = yarp::os::Network::connect(pout,"/icub/left_leg/analog:o");
        if (!b1)
        {
            yWarning("failed input connection");
        }
        if (!b2)
        {
            yWarning("failed ouput connection");
        }
        return true;
    }

    virtual bool close()
    {
        inport.close();
        outport.close();
        return true;
    }

    bool updateModule()
    {
        double verg = 0;

        Bottle bin;
        Bottle bout;
        inport.read(bin);
        if (reset_offset)
        {
            initial_offset[0] = bin.get(0).asDouble();
            initial_offset[1] = bin.get(1).asDouble();
            initial_offset[2] = bin.get(2).asDouble();
            initial_offset[3] = bin.get(3).asDouble();
            initial_offset[4] = bin.get(4).asDouble();
            initial_offset[5] = bin.get(5).asDouble();
            reset_offset = false;
        }

        double x_torque = fabs(bin.get(3).asDouble()) - initial_offset [3];
        double y_force = fabs(bin.get(1).asDouble()) - initial_offset[1];
        bout.addInt(2);
        double j_torque = x_torque + y_force * 0.70; //Nm + N * m
        bout.addDouble(j_torque); //j0
        bout.addDouble(j_torque); //j1
        bout.addDouble(j_torque); //j2
        bout.addDouble(j_torque); //j3
        bout.addDouble(j_torque); //j4
        bout.addDouble(j_torque); //j5
        outport.write(bout);

        return true;
    }

    virtual double getPeriod()
    {
        return 0.010;
    }
};

int main(int argc, char *argv[])
{
    Network yarp;
    VergenceModule mod;
    ResourceFinder rf;
    
    rf.configure(argc, argv);
    rf.setVerbose(true);
    if (mod.configure(rf)==false) return -1;
    mod.runModule();
    yInfo("Main returning\n");
    return 0;

}
