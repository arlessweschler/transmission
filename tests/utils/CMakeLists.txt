function(AddShowTest name file_basename)
   add_test(
      NAME ${name}
      COMMAND ${CMAKE_COMMAND}
      -D transmission_show:string=$<TARGET_FILE:transmission-show>
      -D torrent_file:string=assets/${file_basename}.torrent
      -D reference_file:string=${CMAKE_CURRENT_SOURCE_DIR}/assets/${file_basename}.show
      -P "${CMAKE_CURRENT_SOURCE_DIR}/run_transmission_show.cmake"
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
   )
endfunction()

AddShowTest(transmission-show-ubuntu ubuntu-20.04.3-desktop-amd64.iso)
AddShowTest(transmission-show-thor Thor_and_the_Amazon_Women.avi)
AddShowTest(transmission-show-inner-sanctum Inner_Sanctum_movie_archive)
