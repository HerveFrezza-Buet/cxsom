
struct colormap {
  sked::json::rgb preparer {.5, .5, .7};
  sked::json::rgb readyr   {0., 0., .5};
  sked::json::rgb startr   {.5, .5, 1.};
    
  sked::json::rgb preparew {.7, .5, .5};
  sked::json::rgb readyw   {.5, 0., 0.};
  sked::json::rgb startw   {1., .5, .5};
    
  sked::json::rgb count   {0., .7, 0.};
  sked::json::rgb wait    {.5, .7, .5};
  sked::json::rgb endwait {.2, .4, .2};
    
  sked::json::rgb after   {.8, .8, .3};
  sked::json::rgb finish  {.5, .5, .5};
  
  sked::json::rgb done    {.7, .3, .7};
  sked::json::rgb sync    {.7, .3, .7};
};
