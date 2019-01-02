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
    -- get all objects -- 
    update = false
	h = sim.getObjectAssociatedWithScript(sim.handle_self)
	child = sim.getObjectsInTree(h,sim.object_shape_type , 1)
	if (#child)>0 then
		return
	end
	-- Define xml file -- 
    xml = [[
            <ui closeable="true" resizable="true">
                <label text="GRID DEFINOR" wordwrap="true" />
                    <tabs title="st">
                    <tab title="st">
					<group layout="grid">
						<label text=""/>
						<label text=""/>
						<label text="Max (m)"/>
						<label text="Actual (m)"/>
						<br />
						
						<label text="X size" wordwrap="true" />
						<hslider minimum="1" maximum="100" on-change="sliderChange" id="1"/>
						<edit id="7001" value="0" on-change="boxChange"/>
						<edit id="7002" value="0" on-change="boxChange"/>
						<br />
						
						<label text="Y size" wordwrap="true" />
						<hslider minimum="1" maximum="100" on-change="sliderChange" id="2"/>
						<edit id="7003" value="0" on-change="boxChange"/>
						<edit id="7004" value="0" on-change="boxChange"/>
						<br />
						
						<label text="Z size" wordwrap="true" />
						<hslider minimum="1" maximum="100" on-change="sliderChange" id="3"/>
						<edit id="7005" value="0" on-change="boxChange"/>
						<edit id="7006" value="0" on-change="boxChange"/>
						<br />
						
						<label text="Voxel size" wordwrap="true" />
						<hslider minimum="1" maximum="100" on-change="sliderChange" id="4"/>
						<edit id="7007" value="0" on-change="boxChange"/>
						<edit id="7008" value="0" on-change="boxChange"/>
						<br />
					</group>
					<group layout="form">
                    <label text="Number of voxels : "/>
					<label text="" id="6"/>
					< /group>
                    </tab>
                    </tabs>
            </ui>
          ]]
    ui=simUI.create(xml)
    result,pureType,dimensions=sim.getShapeGeomInfo(h)
	max = {1.5*math.ceil(dimensions[1]), 1.5*math.ceil(dimensions[2]), 1.5*math.ceil(dimensions[3]), math.ceil(1.5*sim.getScriptSimulationParameter(sim.handle_self,"Voxel size"))}
	value = {dimensions[1], dimensions[2], dimensions[3], sim.getScriptSimulationParameter(sim.handle_self,"Voxel size")}
	setColorMapData({4,5},
					{0, 0,
					 1, 0.1,
					 2, 0.1,
					 3, 0.2,
					 0, 0, 0, 1,
					 1 ,0 ,0.5, 0.5,
					 2, 0, 0.8, 0,
					 3, 0.5, 0.5, 0.8,
					 4, 0.8, 0, 0},{},'')
	updateState()
end

function setColorMapData(inInts,inFloats,inStrings,inBuffer)
	colormapDataNum = inInts
	colormapData = inFloats
	return {},{},{},''
end

function getColorMapData(inInts,inFloats,inStrings,inBuffer)
	return colormapDataNum,colormapData,{},''
end

function sysCall_cleanup()
	if (#child)>0 then
		return
	end
    simUI.destroy(ui)
end

function boxChange(ui, id, newVal)
	if (id==7001) then
		case = 1
		index = 1
	elseif (id==7002) then
		case = 2
		index = 1
	elseif (id==7003) then
		case = 1
		index = 2
	elseif (id==7004) then
		case = 2
		index = 2
	elseif (id==7005) then
		case = 1
		index = 3
	elseif (id==7006) then
		case = 2
		index = 3
	elseif (id==7007) then
		case = 1
		index = 4
	elseif (id==7008) then
		case = 2
		index = 4
	end
	
	if (case==1) then
		if (newVal<tostring(value[index])) then
			return
		end
		max[index] = newVal
		updateState()
	elseif (case==2) then
		max[index] = math.max(value[index], max[index])
		sliderChange(ui, index, 100*newVal/max[index])
	end
	
end

function sliderChange(ui,id,newVal)
    result,pureType,dimensions=sim.getShapeGeomInfo(h)
    if (id==1) then
        x = newVal*max[1]/100
        sim.scaleObject(h, x/value[1], 1, 1)
		value[1] = x
    elseif (id==2) then
        y = newVal*max[2]/100
        sim.scaleObject(h, 1, y/value[2], 1)
		value[2] = y
    elseif (id==3) then
        z = newVal*max[3]/100
        sim.scaleObject(h, 1, 1, z/value[3])
        pos = sim.getObjectPosition(h,-1)
        pos[3] = pos[3]+ (z-value[3])/2
        sim.setObjectPosition(h, -1, pos)
		value[3] = z
	elseif (id==4) then
		value[4] = newVal*max[4]/100
    end
	updateState()
end

function updateState()
	simUI.setEditValue(ui, 7001, tostring(max[1]))
	simUI.setEditValue(ui, 7002, tostring(value[1]))
	simUI.setEditValue(ui, 7003, tostring(max[2]))
	simUI.setEditValue(ui, 7004, tostring(value[2]))
	simUI.setEditValue(ui, 7005, tostring(max[3]))
	simUI.setEditValue(ui, 7006, tostring(value[3]))
	simUI.setEditValue(ui, 7007, tostring(max[4]))
	simUI.setEditValue(ui, 7008, tostring(value[4]))
	
	simUI.setSliderValue(ui, 1, math.ceil(100*value[1]/max[1]),true)
    simUI.setSliderValue(ui, 2, math.ceil(100*value[2]/max[2]),true)
    simUI.setSliderValue(ui, 3, math.ceil(100*value[3]/max[3]),true)
	simUI.setSliderValue(ui, 4, 100*value[4]/max[4],true)
	
    vox = math.ceil(value[1]/value[4]) * math.ceil(value[2]/value[4]) * math.ceil(value[3]/value[4])
	simUI.setLabelText(ui,6,tostring(vox))
    sim.setScriptSimulationParameter(sim.handle_self, "Voxel size", value[4])
    update = true
end

function getData(inInts,inFloats,inStrings,inBuffer)
	data = {}
	if (#child)>0 then
		data[1] = child[1]
	else
		data[1] = value[1]/value[4]
		data[2] = value[2]/value[4]
		data[3] = value[3]/value[4]
		data[4] = value[4]
		data[5] = -value[1]/2
		data[6] = -value[2]/2
		data[7] = -value[3]/2
		data[8] = h
	end
    return {}, data, {}, ''
end

function getUpdate(inInts,inFloats,inStrings,inBuffer)
    if (update) then
        data = {1}
        update = false
    else
        data = {}
    end
    return {}, data, {}, ''
end
