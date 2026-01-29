var creativeSize = `${template.width}x${template.height}`; // Set the width and height of the ad.
var devDynamicContent = {}; // Variable for studio invocation code.
function exitCall() {
  Enabler.exitOverride("MainClick", Adlib.utmParser(defaultValues.landingPage+"?utm_source=dynamicSiteNameUTM&utm_medium=paid-display&utm_id=cm|dynamicCampaignIdUTM|dynamicSiteIdUTM|dynamicPlacementIdUTM|dynamicCreativeIdUTM"));
}
var videoCuePoint = [
  //"cuePoint:funcName" ex. "1:firstAninmation" please do not included the parenthesis after the function name.
]
function initDynamic() {
  if (checkEnvironment() === 'tools') {
    for (var i=0;i<Object.keys(defaultValues).length;i++) {
      Object.keys(defaultValues)[i];
    }
  } else {
    // paste studio invocation code here, and delete the devDynamicContent declaration as it is already declared outside this function.
    Enabler.setProfileId(10949086);

    devDynamicContent.HBO_MAX_EMEA_Full_Bleed_LF__Poland_Sheet1 = [{}];
    devDynamicContent.HBO_MAX_EMEA_Full_Bleed_LF__Poland_Sheet1[0]._id = 0;
    devDynamicContent.HBO_MAX_EMEA_Full_Bleed_LF__Poland_Sheet1[0].id = "005ed54f-4d20-45e1-b033-899ed16a7693";
    devDynamicContent.HBO_MAX_EMEA_Full_Bleed_LF__Poland_Sheet1[0].Reporting_Label = "3810c5c4-d28a-43b6-8fb9-3b7c0b50bdc0-c12bb3fc-5105-4920-9049-6fa62e8e2499-b226cfd5-f5fa-49f5-91da-b6e50a786f2a-6i9WvJh2B5-276983c9-86b9-416d-8c48-72d95c08a56e-824b8552c6b5e191f06831908e7f3ca2498db818b3d236c371bd9087944208ea";
    devDynamicContent.HBO_MAX_EMEA_Full_Bleed_LF__Poland_Sheet1[0].Variant_name = "Game of Thrones Generic Display";
    devDynamicContent.HBO_MAX_EMEA_Full_Bleed_LF__Poland_Sheet1[0].Active = true;
    devDynamicContent.HBO_MAX_EMEA_Full_Bleed_LF__Poland_Sheet1[0].isDefault = false;
    devDynamicContent.HBO_MAX_EMEA_Full_Bleed_LF__Poland_Sheet1[0].legal = "";
    devDynamicContent.HBO_MAX_EMEA_Full_Bleed_LF__Poland_Sheet1[0].bgColor = "#000000";
    devDynamicContent.HBO_MAX_EMEA_Full_Bleed_LF__Poland_Sheet1[0].ctaText = "";
    devDynamicContent.HBO_MAX_EMEA_Full_Bleed_LF__Poland_Sheet1[0].subLogo = {};
    devDynamicContent.HBO_MAX_EMEA_Full_Bleed_LF__Poland_Sheet1[0].subLogo.Url = "https://dyle7zu5kwqf5.cloudfront.net/f5618a2b-9b5b-47a6-bb03-73cbcde913b3/blank.png";
    devDynamicContent.HBO_MAX_EMEA_Full_Bleed_LF__Poland_Sheet1[0].trigger = "Display";
    devDynamicContent.HBO_MAX_EMEA_Full_Bleed_LF__Poland_Sheet1[0].cssStyle = "POL";
    devDynamicContent.HBO_MAX_EMEA_Full_Bleed_LF__Poland_Sheet1[0].trigger2 = "HBO Max";
    devDynamicContent.HBO_MAX_EMEA_Full_Bleed_LF__Poland_Sheet1[0].trigger3 = "Static-Dynamic";
    devDynamicContent.HBO_MAX_EMEA_Full_Bleed_LF__Poland_Sheet1[0].baseImage = {};
    devDynamicContent.HBO_MAX_EMEA_Full_Bleed_LF__Poland_Sheet1[0].baseImage.Url = "https://dyle7zu5kwqf5.cloudfront.net/f5618a2b-9b5b-47a6-bb03-73cbcde913b3/fullbleed.png";
    devDynamicContent.HBO_MAX_EMEA_Full_Bleed_LF__Poland_Sheet1[0].uiElement1 = "";
    devDynamicContent.HBO_MAX_EMEA_Full_Bleed_LF__Poland_Sheet1[0].uiElement2 = "";
    devDynamicContent.HBO_MAX_EMEA_Full_Bleed_LF__Poland_Sheet1[0].uiElement3 = "#000000";
    devDynamicContent.HBO_MAX_EMEA_Full_Bleed_LF__Poland_Sheet1[0].uiElement4 = "1";
    devDynamicContent.HBO_MAX_EMEA_Full_Bleed_LF__Poland_Sheet1[0].uiElement5 = "";
    devDynamicContent.HBO_MAX_EMEA_Full_Bleed_LF__Poland_Sheet1[0].ctaBoxColor = "";
    devDynamicContent.HBO_MAX_EMEA_Full_Bleed_LF__Poland_Sheet1[0].frame1Image = {};
    devDynamicContent.HBO_MAX_EMEA_Full_Bleed_LF__Poland_Sheet1[0].frame1Image.Url = "";
    devDynamicContent.HBO_MAX_EMEA_Full_Bleed_LF__Poland_Sheet1[0].landingPage = {};
    devDynamicContent.HBO_MAX_EMEA_Full_Bleed_LF__Poland_Sheet1[0].landingPage.Url = "https://www.hbomax.com/";
    devDynamicContent.HBO_MAX_EMEA_Full_Bleed_LF__Poland_Sheet1[0].CreativeName = "Game of Thrones Generic_POL_Display_300x50";
    devDynamicContent.HBO_MAX_EMEA_Full_Bleed_LF__Poland_Sheet1[0].frame1Image2 = {};
    devDynamicContent.HBO_MAX_EMEA_Full_Bleed_LF__Poland_Sheet1[0].frame1Image2.Url = "https://app.smartly.io/warren/images/679f1d40-c1c4-4216-9c7e-8aebca79dd44/blob?M5KADI_GOTHS8_Banner_FF_POL.png";
    devDynamicContent.HBO_MAX_EMEA_Full_Bleed_LF__Poland_Sheet1[0].URL_Parameter = "";
    devDynamicContent.HBO_MAX_EMEA_Full_Bleed_LF__Poland_Sheet1[0].customVariable = "C154109_POL_GOT_SNG_HBOMR25_SUPN_STATB_300X50_DCO_HBOMAXREBRAND-MAINKADAENERYSPREMIERE";
    devDynamicContent.HBO_MAX_EMEA_Full_Bleed_LF__Poland_Sheet1[0].frame1Headline = "";
    devDynamicContent.HBO_MAX_EMEA_Full_Bleed_LF__Poland_Sheet1[0].customVariable2 = "Game of Thrones Generic Display";
    devDynamicContent.HBO_MAX_EMEA_Full_Bleed_LF__Poland_Sheet1[0].frame1Headline2 = "";
    devDynamicContent.HBO_MAX_EMEA_Full_Bleed_LF__Poland_Sheet1[0].frame1Headline3 = "";
    devDynamicContent.HBO_MAX_EMEA_Full_Bleed_LF__Poland_Sheet1[0].frame1ShapeColor = "#FFFFFF";
    devDynamicContent.HBO_MAX_EMEA_Full_Bleed_LF__Poland_Sheet1[0].frame1Subheadline = "";
    Enabler.setDevDynamicContent(devDynamicContent);
    
    Adlib.assignInvocationCode(); // DO NOT DELETE THIS CODE, This will automatically assign invocation code to defaultValues
  }
}
function populate() {
  //Adlib.preloadDelay = 100;
  //Adlib.fpsSettings(60); // uncomment this if you want to change the FPS used in the creative          
  Adlib.populateElements(); // DO NOT DELETE THIS. automatically assign the defaultValues to the elements within the ad.
  /*****************************************
  If you need to manually assign a defaultValue to a style of an element, add it below.
  *****************************************/
};