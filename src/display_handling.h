/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   display_handling.h                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: roaleksa <r.aleksandroff@gmail.com>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/22 10:00:00 by roaleksa          #+#    #+#             */
/*   Updated: 2026/08/22 15:00:00 by roaleksa         ###   ########.fr       */
/*                                                                            */
/*                                                                            */
/*   For use with display_handling.cpp only! Do not include elsewhere.        */
/*                                                                            */
/* ************************************************************************** */

#ifndef DISPLAY_HANDLING_H
# define DISPLAY_HANDLING_H

typedef enum {
    DISPLAY_VIEW_INVALID,
    DISPLAY_VIEW_BLANK,
    DISPLAY_VIEW_CLUSTER,
    DISPLAY_VIEW_EXAM
} DISPLAY_VIEW_t;

typedef enum {
    CLUSTER_SIDE_NONE,
    CLUSTER_SIDE_DEFAULT,
    CLUSTER_SIDE_INTRA_ERROR,
    CLUSTER_SIDE_SECRET_EXPIRED,
    CLUSTER_SIDE_EXAM_DAY,
    CLUSTER_SIDE_LOW_BATTERY,
    CLUSTER_SIDE_OTA_WAITING,
    CLUSTER_SIDE_OTA_SUCCESS,
    CLUSTER_SIDE_OTA_FAIL,
    CLUSTER_SIDE_OTA_CANCELED,
    CLUSTER_SIDE_TELEGRAM_ERROR
} CLUSTER_SIDE_t;

typedef enum {
    EXAM_SCREEN_NONE,
    EXAM_SCREEN_PRE_50,
    EXAM_SCREEN_PRE_25,
    EXAM_SCREEN_PRE_5,
    EXAM_SCREEN_ACTIVE
} EXAM_SCREEN_t;

typedef struct {
    DISPLAY_VIEW_t view;
    CLUSTER_SIDE_t cluster_side;
    EXAM_SCREEN_t  exam_screen;
    uint16_t       exam_hour;
    uint16_t       exam_minutes;
} DISPLAY_STATE_t;

RTC_DATA_ATTR static bool            display_cache_valid;
RTC_DATA_ATTR static DISPLAY_STATE_t display_cache;

#endif
 