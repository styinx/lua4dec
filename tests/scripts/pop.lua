
-- attract mode screen, plays a series of movies

-- Helper function, stops movie, returns
function ifs_attract_fnStopAndReturn(this)
    if(this.bMoviePlaying) then
        this.bMoviePlaying = nil
        ifelem_shellscreen_fnStopMovie()
        ScriptCB_PopScreen()
    end
end


ifs_attract = NewIFShellScreen {
    Update = function(this, fDt)

        this.timeout = this.timeout - fDt
        if (this.timeout < 0) then
            ifs_attract_fnStopAndReturn(this)
        end

        -- Do periodic check if controllers are present.
        this.fControllerCheck = this.fControllerCheck - fDt
        if(this.fControllerCheck < 0) then
            this.fControllerCheck = 0.25

            local iNumControllers = ScriptCB_GetNumControllers()
            if(this.iLastControllers ~= iNumControllers) then
                this.iLastControllers = iNumControllers

                if((iNumControllers > 0) or (gPlatformStr ~= "PS2")) then
                    IFText_fnSetString(this.title,"game.attractmode.title")
                else
                    IFText_fnSetString(this.title,"ifs.start.nocontroller")
                end
            end
        end
    end,
}

-- add screen to GUI manager
AddIFScreen(ifs_attract, "ifs_attract")
