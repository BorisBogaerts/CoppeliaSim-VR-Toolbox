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
	useAdvancedDialog = sim.getScriptSimulationParameter(sim.handle_self, "advancedDialog")
	useCustomQualityFunction = sim.getScriptSimulationParameter(sim.handle_self, "useCustomQualityFunction")
	shaderCode = sim.getScriptSimulationParameter(sim.handle_self, "shaderCode")
	qualityThreshold = sim.getScriptSimulationParameter(sim.handle_self, "qualityThreshold")
	integrateMeasurement = sim.getScriptSimulationParameter(sim.handle_self, "integrateMeasurement")
	
	-- Define used variables
    h = sim.getObjectHandle('Camera_feeder')
    cams=sim.getObjectsInTree(h,sim.object_visionsensor_type)
	prevTrackpad = false
	
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
						<tab title="Measurement quality">
							<group layout="form">
								<label text="Use custom quality function (true -> custom quality, false -> binary coverage): "/>
								<checkbox checked="true" text="" id="4" on-change="getValues"/>
								
								<label text="Integrate measurement:"/>
								<checkbox  checked="false" text="" id="8" on-change="getValues"/>
								
								<label text="Quality threshold: "/>
								<edit  value="1" id="5" on-change="getValues"/>
								
								<label text="Custom quality function as shader code (ca -> cos(incidence angle), di -> measurement distance, must be valid shader code): "/>
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
	useCustomQualityFunction = sim.getScriptSimulationParameter(sim.handle_self, "useCustomQualityFunction")
	shaderCode = sim.getScriptSimulationParameter(sim.handle_self, "shaderCode")
	qualityThreshold = sim.getScriptSimulationParameter(sim.handle_self, "qualityThreshold")
	integrateMeasurement = sim.getScriptSimulationParameter(sim.handle_self, "integrateMeasurement")
	
	simUI.setCheckboxValue(ui, 4, bool_to_number(useCustomQualityFunction))
	simUI.setEditValue(ui, 5, tostring(qualityThreshold))
	simUI.setEditValue(ui, 6, shaderCode)
	simUI.setCheckboxValue(ui, 8, bool_to_number(integrateMeasurement))
end

function getValues(ui, id, newVal)
	local persistent =  (simUI.getCheckboxValue(ui, 7)==2)
	useCustomQualityFunction = (simUI.getCheckboxValue(ui, 4)==2)
	qualityThreshold = tonumber(simUI.getEditValue(ui, 5))
	shaderCode = simUI.getEditValue(ui, 6)
	integrateMeasurement = (simUI.getCheckboxValue(ui, 8)==2)
	if (persistent) then
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