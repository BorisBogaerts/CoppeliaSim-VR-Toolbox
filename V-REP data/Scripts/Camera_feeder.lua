-- Copyright (c) 2018, Boris Bogaerts
-- All rights reserved.

-- Redistribution and use in source and binary forms, with or without 
-- modification, are permitted provided that the following conditions 
-- are met:

-- 1. Redistributions of source code must retain the above copyright 
-- notice, this list of conditions and the following disclaimer.

-- 2. Redistributions in binary form must reproduce the above copyright 
-- notice, this list of conditions and the following disclaimer in the 
-- documentation and/or other materials provided with the distribution.

-- 3. Neither the name of the copyright holder nor the names of its 
-- contributors may be used to endorse or promote products derived from 
-- this software without specific prior written permission.

-- THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
-- "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
-- LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR 
-- A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
-- HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
-- SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
-- LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
-- DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
-- THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
-- (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
-- OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

function sysCall_init()
	-- Get various script parameters
    minPickDistance = sim.getScriptSimulationParameter(sim.handle_self, "minPickDistance")
	trace = sim.getScriptSimulationParameter(sim.handle_self, "tracing")
	useAdvancedDialog = sim.getScriptSimulationParameter(sim.handle_self, "advancedDialog")
	traceDist = sim.getScriptSimulationParameter(sim.handle_self, "traceDistance")
	pick = sim.getScriptSimulationParameter(sim.handle_self, "useStandardPicker")
	useCustomQualityFunction = sim.getScriptSimulationParameter(sim.handle_self, "useCustomQualityFunction")
	shaderCode = sim.getScriptSimulationParameter(sim.handle_self, "shaderCode")
	qualityThreshold = sim.getScriptSimulationParameter(sim.handle_self, "qualityThreshold")
	integrateMeasurement = sim.getScriptSimulationParameter(sim.handle_self, "integrateMeasurement")
	
	-- Define used variables
	path = nil
	vive = sim.getObjectHandle("HTC_VIVE")
    h = sim.getObjectHandle('Camera_feeder')
    cams=sim.getObjectsInTree(h,sim.object_visionsensor_type)
	print(cams)
	prevTrackpad = false
	Elapsedtime = nil
	
	-- Chack for cameras (no cams no measurement
	if (#cams==0) then
		print("WARNING : No cameras detected in Camera_feeder")
		return
	end
    h_h = sim.getObjectHandle('Cam_handle')
    h_f = sim.getObjectHandle('Cam_feed')
    h_c = sim.getObjectHandle('Cam')
    t1_h = sim.getObjectPosition(h_h, h_c)
    t2_h = sim.getObjectPosition(h_f, h_c)

    o1_h = sim.getObjectOrientation(h_h, h_c)
    o2_h = sim.getObjectOrientation(h_f, h_c)
    prevTrigL = 0
    prevTrigR = 0
    handles = {}
    feeds = {}
	
	-- Initialize signals
	sim.setIntegerSignal('ResetMeasurement',0) -- set up string signal to transfer button state
	sim.setIntegerSignal('MeasurementInProgress',0) -- set up string signal to transfer button state
	
	-- replicate the template for each camera
    for i = 1, #cams, 1 do 
        h_h_c = sim.copyPasteObjects({h_h},0)
        h_h_c = h_h_c[1]
        handles[#handles+1] = h_h_c
        sim.setObjectPosition(h_h_c, cams[i], t1_h)
        sim.setObjectOrientation(h_h_c, cams[i], o1_h)
        sim.setObjectParent(h_h_c, h, true)
        sim.setObjectParent(cams[i], h_h_c, true)
        
        h_f_c = sim.copyPasteObjects({h_f},0)
        h_f_c = h_f_c[1]
        feeds[#feeds+1] = h_f_c
        sim.setObjectPosition(h_f_c, cams[i], t2_h)
        sim.setObjectOrientation(h_f_c, cams[i], o2_h)
        sim.setObjectParent(h_f_c, cams[i], true)
		resolution=sim.getVisionSensorResolution(cams[i])       
        local a = math.min(1, resolution[1]/resolution[2])
        local b = math.min(1, resolution[2]/resolution[1])
        sim.scaleObject(h_f_c, a, b, 1, 0)
    end
	
	-- Build gui if asked
	if (useAdvancedDialog) then
		xml = [[
				<ui closeable="true" resizable="true">
					<label text="Advanced camera quality properties editor" wordwrap="true" />
					
					<group layout="form">
						<label text="Changes are persistent: "/>
						<checkbox checked="false" text="" id="7" on-change="getValues"/>
					</group>
					 <tabs>
						<tab title="Interaction">
							<group layout="form">
								<label text="Use standard picker (drag cameras): "/>
								<checkbox checked="true" text="" id="1" on-change="getValues"/>
								
								<label text="Trace line: "/>
								<checkbox checked="true" text="" id="2" on-change="getValues"/>
								
								<label text="Trace distance (distance between points): "/>
								<edit  value="0.1" id="3" on-change="getValues"/>
							</group>
						</tab>
						
						<tab title="Measurement quality">
							<group layout="form">
								<label text="Use quality function (true -> custom quality, false -> binary coverage): "/>
								<checkbox checked="true" text="" id="4" on-change="getValues"/>
								
								<label text="Integrate measurement:"/>
								<checkbox  checked="false" text="" id="8" on-change="getValues"/>
								
								<label text="Quality threshold: "/>
								<edit  value="1" id="5" on-change="getValues"/>
								
								<label text="Shader function (ca -> cos(incidence angle), di -> measurement distance, must be valid shader code): "/>
								<edit  value="ca/pow(di,2);" id="6" on-change="getValues"/>
							</group>
						</tab>
					  </tabs>
				</ui>
			  ]]
		ui=simUI.create(xml)
		setValues()
	end
end

function setValues()
	trace = sim.getScriptSimulationParameter(sim.handle_self, "tracing")
	traceDist = sim.getScriptSimulationParameter(sim.handle_self, "traceDistance")
	useCustomQualityFunction = sim.getScriptSimulationParameter(sim.handle_self, "useCustomQualityFunction")
	shaderCode = sim.getScriptSimulationParameter(sim.handle_self, "shaderCode")
	qualityThreshold = sim.getScriptSimulationParameter(sim.handle_self, "qualityThreshold")
	integrateMeasurement = sim.getScriptSimulationParameter(sim.handle_self, "integrateMeasurement")
	
	simUI.setCheckboxValue(ui, 2, bool_to_number(trace))
	simUI.setEditValue(ui, 3, tostring(traceDist))
	simUI.setCheckboxValue(ui, 4, bool_to_number(useCustomQualityFunction))
	simUI.setEditValue(ui, 5, tostring(qualityThreshold))
	simUI.setEditValue(ui, 6, shaderCode)
	simUI.setCheckboxValue(ui, 8, bool_to_number(integrateMeasurement))
end

function getValues(ui, id, newVal)
	local persistent =  (simUI.getCheckboxValue(ui, 7)==2)
	trace = (simUI.getCheckboxValue(ui, 2)==2)
	traceDist = tonumber(simUI.getEditValue(ui, 3))
	useCustomQualityFunction = (simUI.getCheckboxValue(ui, 4)==2)
	qualityThreshold = tonumber(simUI.getEditValue(ui, 5))
	shaderCode = simUI.getEditValue(ui, 6)
	integrateMeasurement = (simUI.getCheckboxValue(ui, 8)==2)
	if (persistent) then
	    sim.setScriptSimulationParameter(sim.handle_self, "traceDistance", traceDist)
		sim.setScriptSimulationParameter(sim.handle_self, "useStandardPicker", tostring(pick))
		sim.setScriptSimulationParameter(sim.handle_self, "useCustomQualityFunction", tostring(useCustomQualityFunction))
		sim.setScriptSimulationParameter(sim.handle_self, "shaderCode", shaderCode)
		sim.setScriptSimulationParameter(sim.handle_self, "qualityThreshold", qualityThreshold)
		sim.setScriptSimulationParameter(sim.handle_self, "integrateMeasurement", tostring(integrateMeasurement))
	end
end

function bool_to_number(value)
  return value and 2 or 0
end

function sysCall_cleanup()
	if (#cams==0) then
		return
	end
	if (useAdvancedDialog) then
		simUI.destroy(ui)
	end
end

function sysCall_sensing()
    if (#cams==0) then
		return
	end
	
	if (sim.getIntegerSignal("R_Trigger_Press")==1) then
		sim.setIntegerSignal('MeasurementInProgress',1) -- set up string signal to transfer button state
	else
		sim.setIntegerSignal('MeasurementInProgress',0) -- set up string signal to transfer button state
	end
	
	if(integrateMeasurement) then
		--sim.setIntegerSignal('ResetMeasurement',0) -- set up string signal to transfer button state
		--sim.setIntegerSignal('MeasurementInProgress',sim.getIntegerSignal("RightControllerTrigger")) -- set up string signal to transfer button state
		checkReset()
		if (sim.getIntegerSignal("MeasurementInProgress")==0) then
			return
		end
	end
	
	if (trace) then
		doTrace()
	end
end

function checkReset()
	local tpad = sim.getIntegerSignal("L_TrackPad_Press")==1
	if (tpad==false) then -- if not pressed return
		prevTrackpad = false
		return
	end
	if (prevTrackpad==false) then -- first time after press
		Elapsedtime = sim.getSimulationTime()		-- record starting time
		prevTrackpad = true
		return
	end
	
	if (sim.getSimulationTime()-Elapsedtime)>=5 then -- if the elapsed time is more than five seconds
		sim.setIntegerSignal("ResetMeasurement",1) -- only now send reset pulse
		Elapsedtime = sim.getSimulationTime()
		path = nil
		--sim.setStringSignal('path', sim.packFloatTable({0,0,0}))
		print("Reset")
	end
	prevTrackpad = tpad
end

function doTrace()
	if (path==nil) then
		path = sim.getObjectPosition(feeds[1], vive)
		path[#path+1] = path[1]
		path[#path+1] = path[2]
		path[#path+1] = path[3]
	else
		pos = sim.getObjectPosition(feeds[1], vive)
		if (pointDistance(pos, {path[#path-2], path[#path-1], path[#path]}) < traceDist) then
		sim.setStringSignal('path', sim.packFloatTable(path))
			return
		end
		path[#path+1] = pos[1]
		path[#path+1] = pos[2]
		path[#path+1] = pos[3]
	end
	sim.setStringSignal('path', sim.packFloatTable(path))
end

function pointDistance(p1,p2)
	local pointDist = math.sqrt((p1[1] - p2[1])^2 + (p1[2] - p2[2])^2  + (p1[3] - p2[3])^2)
	return pointDist
end

function helloPath(inInts,inFloats,inStrings,inBuffer)
	ret = {}
	if (trace) then
		ret[1] = 1
	else
		ret[1] = 0
	end
	return ret, {}, {}, ''
end

function getCustomShaderCode(inInts,inFloats,inStrings,inBuffer)
	return {}, {}, {shaderCode}, ''
end

function getQualityMode(inInts,inFloats,inStrings,inBuffer)
	return {bool_to_number(useCustomQualityFunction), bool_to_number(integrateMeasurement)}, {qualityThreshold}, {}, ''
end

function getData(inInts,inFloats,inStrings,inBuffer)
    data = {}
    for i = 1, #cams, 1 do
        data[#data + 1] = cams[i]
        data[#data + 1] = handles[i]
        data[#data + 1] = feeds[i]
        result,perspAngle=sim.getObjectFloatParameter(cams[i], sim.visionfloatparam_perspective_angle)
        result,nearClip=sim.getObjectFloatParameter(cams[i], sim.visionfloatparam_near_clipping)
        result,farClip=sim.getObjectFloatParameter(cams[i], sim.visionfloatparam_far_clipping)
        resolution=sim.getVisionSensorResolution(cams[i]) 
        result,pureType,dimensions=sim.getShapeGeomInfo(feeds[i])
        data[#data + 1] = resolution[1]*sim.getScriptSimulationParameter(sim.handle_self, "scaling")
        data[#data + 1] = resolution[2]*sim.getScriptSimulationParameter(sim.handle_self, "scaling")
        data[#data + 1] = perspAngle
        data[#data + 1] = nearClip
        data[#data + 1] = farClip
        data[#data + 1] = dimensions[1]
        data[#data + 1] = dimensions[2]
    end
    vol = sim.getObjectHandle("Field")
    return {}, data, {}, ''
end

function getData(inInts,inFloats,inStrings,inBuffer)
    data = {}
    for i = 1, #cams, 1 do
        data[#data + 1] = cams[i]
        data[#data + 1] = handles[i]
        data[#data + 1] = feeds[i]
        result,perspAngle=sim.getObjectFloatParameter(cams[i], sim.visionfloatparam_perspective_angle)
        result,nearClip=sim.getObjectFloatParameter(cams[i], sim.visionfloatparam_near_clipping)
        result,farClip=sim.getObjectFloatParameter(cams[i], sim.visionfloatparam_far_clipping)
        resolution=sim.getVisionSensorResolution(cams[i]) 
        result,pureType,dimensions=sim.getShapeGeomInfo(feeds[i])
        data[#data + 1] = resolution[1]*sim.getScriptSimulationParameter(sim.handle_self, "scaling")
        data[#data + 1] = resolution[2]*sim.getScriptSimulationParameter(sim.handle_self, "scaling")
        data[#data + 1] = perspAngle
        data[#data + 1] = nearClip
        data[#data + 1] = farClip
        data[#data + 1] = dimensions[1]
        data[#data + 1] = dimensions[2]
    end
    return {}, data, {}, ''
end

function getPose(inInts, inFloats, inStrings,inBuffer)
    data=sim.getObjectMatrix(inInts[1],-1)
    return {}, data, {}, ''
end

-- boven-> achter // onder->boven