function resizeContent() {
    // this is awful code, but I have no other solution

    var body_height = $('body').outerHeight()
    //console.log("Body height: " + body_height)
    var header_height = $('#leaderboard-header').outerHeight()
    var header_margin_top = parseInt($('#leaderboard-header').css('margin-top'), 10)
    var header_margin_bottom = parseInt($('#leaderboard-header').css('margin-bottom'), 10)
    //console.log("Header height: " + header_height)
    //console.log("Header margin top: " + header_margin_top)
    //console.log("Header margin bottom: " + header_margin_bottom)

    $('#leaderboard-content').height(body_height - header_height - header_margin_top - header_margin_bottom)

    var leaderboards_list_height = $('#leaderboards-list').outerHeight()
    //console.log("Leaderboard list height: " + leaderboards_list_height)

    var leaderboards_filter_height = $('#leaderboards-filter').outerHeight()
    var leaderboards_filter_margin_top = parseInt($('#leaderboards-filter').css('margin-top'), 10)
    //console.log("Leaderboard filter height: " + leaderboards_filter_height)
    //console.log("Leaderboard filter margin top: " + leaderboards_filter_margin_top)
    
    $('#leaderboards-list-items').height(leaderboards_list_height - leaderboards_filter_height - leaderboards_filter_margin_top)

    var user_leaderboard_height = $('#user-leaderboard').outerHeight()
    //console.log("User leaderboard height: " + user_leaderboard_height)

    var leaderboard_pagination_height = $('#user-leaderboard-pagination').outerHeight()
    var leaderboard_pagination_margin_top = parseInt($('#user-leaderboard-pagination').css('margin-top'), 10)
    var leaderboard_pagination_margin_bottom = parseInt($('#user-leaderboard-pagination').css('margin-bottom'), 10)
    //console.log("Leaderboard pagination height: " + leaderboard_pagination_height)
    //console.log("Leaderboard pagination margin top: " + leaderboard_pagination_margin_top)
    //console.log("Leaderboard pagination margin bottom: " + leaderboard_pagination_margin_bottom)

    var leaderboard_time_height = $('#user-leaderboard-time').outerHeight()
    var leaderboard_time_margin_top = parseInt($('#user-leaderboard-time').css('margin-top'), 10)
    var leaderboard_time_margin_bottom = parseInt($('#user-leaderboard-time').css('margin-bottom'), 10)
    //console.log("Leaderboard time height: " + leaderboard_time_height)
    //console.log("Leaderboard time margin top: " + leaderboard_time_margin_top)
    //console.log("Leaderboard time margin bottom: " + leaderboard_time_margin_bottom)

    $('#user-leaderboard-table-div').height(
        user_leaderboard_height
        - leaderboard_pagination_height
        - leaderboard_pagination_margin_top
        - leaderboard_pagination_margin_bottom
        - leaderboard_time_height
        - leaderboard_time_margin_top
        - leaderboard_time_margin_bottom)
    $('#user-leaderboard-table').height($('#user-leaderboard-table-div').height())

}

$(document).ready(resizeContent);
$(window).resize(resizeContent);