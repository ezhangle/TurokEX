//-----------------------------------------------------------------------------
//
// Hud.js
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

Hud = class.define(function()
{
    this.canvas = new Canvas();
});

class.properties(Hud,
{
    //------------------------------------------------------------------------
    // VARS
    //------------------------------------------------------------------------
    
    canvas : null,
    
    //------------------------------------------------------------------------
    // FUNCTIONS
    //------------------------------------------------------------------------
    
    onTick : function() { },
    onDraw : function() { },
    start : function() { }
});
